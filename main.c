#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include "zlib/include/z_file.h"
#include "zlib/include/z_string.h"
#include "zlib/include/z_hash_table.h"
#include "zlib/include/z_heap.h"
#include "zlib/include/z_path.h"
#include "zlib/include/z_scanner.h"
#include "zlib/include/z_error.h"
#include "zlib/include/z_env.h"
#include "zlib/include/z_time.h"

typedef struct {
    Z_Heap *heap;
    Z_Hash_Table *table;
} Parse_Desktop_File_State;

void remove_field_codes(Z_String *command)
{
    z_str_replace(command, z_sv("%f"), z_sv(""));
    z_str_replace(command, z_sv("%F"), z_sv(""));
    z_str_replace(command, z_sv("%u"), z_sv(""));
    z_str_replace(command, z_sv("%U"), z_sv(""));
    z_str_replace(command, z_sv("%i"), z_sv(""));
    z_str_replace(command, z_sv("%c"), z_sv(""));
    z_str_replace(command, z_sv("%k"), z_sv(""));
    z_str_trim(command);
}

bool process_desktop_file(Parse_Desktop_File_State *state, const char *pathname)
{
    Z_Heap_Auto scratch = {0};
    FILE *fp = fopen(pathname, "r");

    if (fp == NULL) {
        return false;
    }

    Z_String line = z_str_new(&scratch, "");
    Z_String name = z_str_new(&scratch, "");
    Z_String exec = z_str_new(&scratch, "");

    while(z_file_read_line(fp, &line) && !(name.length && exec.length)) {
        Z_String_View line_sv = z_sv(line);

        if (z_sv_like(line_sv, z_sv("Name=%")) && name.length == 0) {
            z_str_append_str(&name, z_sv_trim(z_sv_split_part(line_sv, z_sv("="), 1)));
        } else if (z_sv_like(line_sv, z_sv("Exec=%")) && exec.length == 0) {
            z_str_append_str(&exec, z_sv_trim(z_sv_split_part(line_sv, z_sv("="), 1)));
        }
    }

    fclose(fp);

    if (name.length && exec.length) {
        char *key = z_str_to_cstr(z_str_new_from_sv(state->heap, z_sv(name)));
        char *value = z_str_to_cstr(z_str_new_from_sv(state->heap, z_sv(exec)));
        z_hash_table_put(state->table, key, value, NULL);
        return true;
    }

    return false;
}

bool is_desktop_file(Z_String_View path)
{
    return z_sv_ends_with(path, z_sv(".desktop"));
}

bool fetch_desktop_files_from_directory(Parse_Desktop_File_State *state, const char *pathname)
{
    Z_Heap_Auto scratch = {0};
    DIR *dir = opendir(pathname);

    if (!dir) {
        return false;
    }

    Z_String full_path = z_str_new(&scratch, "");
    struct dirent *entry;

    while ((entry = readdir(dir))) {
        z_str_set_format(&full_path, "%s/%s", pathname, entry->d_name);

        if (is_desktop_file(z_sv(full_path))) {
            process_desktop_file(state, z_str_to_cstr(full_path));
        }
    }

    closedir(dir);

    return true;
}

void fetch_desktop_files(Parse_Desktop_File_State *state)
{
    Z_Clock start = z_get_clock();

    const char *dirs = z_try_get_env("XDG_DATA_DIRS", NULL);

    if (dirs == NULL) {
        z_die("XDG_DATA_DIRS is not defined\n");
    }

    Z_Heap_Auto scratch = {0};
    Z_Sv_Split_Iter iter = z_sv_split(z_sv(dirs), z_sv(":"));
    Z_String_View dir;
    Z_String dir_str = z_str_new(&scratch, "");

    while (z_sv_split_next(&iter, &dir)) {
        z_str_set_format(&dir_str, "%.*s/applications", dir.length, dir.ptr);
        fetch_desktop_files_from_directory(state, z_str_to_cstr(dir_str));
    }

    double ms = z_clock_get_elapsed_mseconds(start);
    printf("Loaded %zu apps in %.1lfms\n", state->table->size, ms);
}

char **build_dmenu_args(Z_Heap *heap, int argc, char **argv)
{
    char **dmenu = z_heap_malloc(heap, sizeof(char *) * argc + 1);

    dmenu[0] = z_cstr_dup(heap, "dmenu");

    for (int i = 1; i < argc; i++) {
        dmenu[i] = z_cstr_dup(heap, argv[i]);
    }

    dmenu[argc] = NULL;

    return dmenu;
}

void print_dmenu_command(int argc, char **dmenu)
{
    printf("dmenu: ");

    for (int i = 0; i < argc - 1; i++) {
        printf("%s ", dmenu[i]);
    }

    printf("%s\n", dmenu[argc - 1]);
}

void pipe_program_names(Parse_Desktop_File_State *state, FILE *fp)
{
    Z_Hash_Table_Iter iter = z_hash_table_iter(state->table);
    Z_Pair pair;

    while (z_hash_table_iter_next(&iter, &pair)) {
        fprintf(fp, "%s\n", (char *)pair.key);
    }
}

int execute_program(const Parse_Desktop_File_State *state, const char *program_name)
{
    const char *exec = z_hash_table_get(state->table, program_name);

    if (exec == NULL) {
        z_die("No command found for name: \"%s\"\n", program_name);
    }

    Z_String command = z_str_new(state->heap, "%s", exec);
    remove_field_codes(&command);
    printf("Running: %s\n", command.ptr);

    int pid = fork();

    if (pid == -1) {
        z_die("fork() failed\n");
    }

    if (pid == 0) {
        return system(command.ptr);
    }

    return 0;
}

int main(int argc, char **argv)
{
    Z_Heap_Auto heap = {0};
    Z_Hash_Table table = z_hash_table_new(&heap, z_str_equal, z_str_hash);

    Parse_Desktop_File_State state = {
        .heap = &heap,
        .table = &table,
    };

    fetch_desktop_files(&state);

    char **dmenu_args = build_dmenu_args(state.heap, argc, argv);
    print_dmenu_command(argc, dmenu_args);
    Z_Piped_Process dmenu = z_pipe_process(dmenu_args, Z_Redirect_Stdout | Z_Redirect_Stdin);
    pipe_program_names(&state, dmenu.stdin);
    fclose(dmenu.stdin);

    Z_String selected_program = z_str_new(&heap, "");
    z_file_read_line(dmenu.stdout, &selected_program);
    z_str_trim(&selected_program);
    fclose(dmenu.stdout);

    if (selected_program.length == 0) {
        printf("Cancel.\n");
        return 0;
    }

    printf("Selected: %s\n", selected_program.ptr);
    execute_program(&state, selected_program.ptr);

    return 0;
}
