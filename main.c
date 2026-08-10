#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include "./zlib/include/z_file.h"
#include "./zlib/include/z_string.h"
#include "./zlib/include/z_hash_table.h"
#include "./zlib/include/z_heap.h"
#include "./zlib/include/z_path.h"
#include "./zlib/include/z_scanner.h"
#include "./zlib/include/z_error.h"
#include "./zlib/include/z_env.h"
#include <linux/limits.h>

typedef struct {
    Z_Heap *heap;
    Z_Hash_Table *table;
} Parse_Desktop_File_State;

typedef struct {
    Z_String name;
    Z_String exec;
    Z_String icon;
    Z_String absolute_path;
} Desktop_File;

Desktop_File *make_desktop_file(Z_Heap *heap, Z_String_View name, Z_String_View exec, Z_String_View icon, Z_String_View absolute_path)
{
    Desktop_File *desktop_file = z_heap_malloc(heap, sizeof(Desktop_File));
    desktop_file->name = z_str_new_from_sv(heap, name);
    desktop_file->exec = z_str_new_from_sv(heap, exec);
    desktop_file->icon = z_str_new_from_sv(heap, icon);
    desktop_file->absolute_path = z_str_new_from_sv(heap, absolute_path);

    return desktop_file;
}

bool proccess_desktop_file(Parse_Desktop_File_State *state, const char *pathname)
{
    Z_Heap_Auto scratch = {0};
    FILE *fp = fopen(pathname, "r");

    if (fp == NULL) {
        return false;
    }

    Z_String line = z_str_new(&scratch, "");
    Z_String name = z_str_new(&scratch, "");
    Z_String exec = z_str_new(&scratch, "");
    Z_String icon = z_str_new(&scratch, "");

    while(z_file_read_line(fp, &line) && !(name.length && exec.length && icon.length)) {
        Z_String_View line_sv = z_sv(line);

        if (z_sv_like(line_sv, z_sv("Name=%")) && name.length == 0) {
            z_str_append_str(&name, z_sv_trim(z_sv_split_part(line_sv, z_sv("="), 1)));
        } else if (z_sv_like(line_sv, z_sv("Exec=%")) && exec.length == 0) {
            z_str_append_str(&exec, z_sv_trim(z_sv_split_part(line_sv, z_sv("="), 1)));
        } else if (z_sv_like(line_sv, z_sv("Icon=%")) && icon.length == 0) {
            z_str_append_str(&icon, z_sv_trim(z_sv_split_part(line_sv, z_sv("="), 1)));
        }

        z_str_clear(&line);
    }

    z_str_trim(&name);
    z_str_trim(&exec);
    z_str_trim(&icon);

    
    if (name.length && exec.length) {
        char absolute_path[PATH_MAX];
        realpath(pathname, absolute_path);
        Desktop_File *desktop_File = make_desktop_file(state->heap, z_sv(name), z_sv(exec), z_sv(icon), z_sv(absolute_path));
        z_hash_table_put(state->table, z_str_to_cstr(name), desktop_File, NULL);
    }

    fclose(fp);
    return true;
}

bool is_desktop_file(Z_String_View path)
{
    return z_sv_ends_with(path, z_sv(".desktop"));
}

bool proccess_directory(Parse_Desktop_File_State *state, const char *pathname)
{
    Z_Heap_Auto scratch = {0};
    DIR *dir = opendir(pathname);

    if (!dir) {
        z_perror_format("opendir('%s')", pathname);
        return false;
    }

    Z_String full_path = z_str_new(&scratch, "");
    struct dirent *entry;

    while ((entry = readdir(dir))) {
        z_str_append_format(&full_path, "%s/%s", pathname, entry->d_name);

        if (is_desktop_file(z_sv(full_path))) {
            proccess_desktop_file(state, z_str_to_cstr(full_path));
        }

        z_str_clear(&full_path);
    }

    closedir(dir);

    return true;
}

void proccess_directories(Parse_Desktop_File_State *state)
{
    const char *dirs = z_try_get_env("XDG_DATA_DIRS", NULL);

    if (dirs == NULL) {
        z_die("XDG_DATA_DIRS is not defined\n");
    }

    Z_Heap_Auto scratch = {0};
    Z_Sv_Split_Iterator iter = z_sv_split(z_sv(dirs), z_sv(":"));
    Z_String_View dir;
    Z_String dir_str = z_str_new(&scratch, "");

    while (z_sv_split_next(&iter, &dir)) {
        z_str_append_str(&dir_str, dir);
        z_str_append_cstr(&dir_str, "/applications");
        proccess_directory(state, z_str_to_cstr(dir_str));
        z_str_clear(&dir_str);
    }
}

// `firefox %F` -> `firefox`
void remove_field_codes(Z_String *command, Z_String_View name, Z_String_View icon)
{
    Z_Heap_Auto heap = {0};

    z_str_replace(command, z_sv("%f"), z_sv(""));
    z_str_replace(command, z_sv("%F"), z_sv(""));
    z_str_replace(command, z_sv("%u"), z_sv(""));
    z_str_replace(command, z_sv("%U"), z_sv(""));
    z_str_replace(command, z_sv("%c"), name);

    // real path
    z_str_replace(command, z_sv("%k"), z_sv(""));


    if (icon.length) {
        Z_String icon_argument = z_str_new(&heap, "--icon %.*s", icon.length, icon.ptr);
        z_str_replace(command, z_sv("%i"), z_sv(icon_argument));
    } else {
        z_str_replace(command, z_sv("%i"), z_sv(""));
    }
}

void print_desktop_file(Desktop_File *desktop_File)
{
    printf("{ Name: '");
    z_sv_print(z_sv(desktop_File->name));
    printf("', Exec: '");
    z_sv_print(z_sv(desktop_File->exec));
    printf("', Icon: '");
    z_sv_print(z_sv(desktop_File->icon));
    printf("', absolutePathname: '");
    z_sv_print(z_sv(desktop_File->absolute_path));
    printf("' }\n");
}

int main()
{
    Z_Heap_Auto heap = {0};
    Z_Hash_Table table = z_hash_table_new(&heap, z_str_equal, z_str_hash);

    Parse_Desktop_File_State state = {
        .heap = &heap,
        .table = &table,
    };

    proccess_directories(&state);

    Z_Pair_Array pairs = z_hash_table_to_array(&heap, &table);
    printf("%zu\n", table.size);

    for (size_t i = 0; i < pairs.length; i++) {
        print_desktop_file(pairs.ptr[i].value);
    }

    return 0;
}
