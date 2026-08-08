#include <stdio.h>
#include <dirent.h>
#include "./zlib/include/z_file.h"
#include "./zlib/include/z_string.h"
#include "./zlib/include/z_hash_table.h"
#include "./zlib/include/z_heap.h"
#include "./zlib/include/z_path.h"
#include "./zlib/include/z_scanner.h"

const char *DESKTOP_FILES_DIRECTORIES[] = {
    "/usr/share/applications",
    "~/.local/share/applications",
    "/var/lib/flatpak/exports/share/applications",
    NULL,
};

typedef struct {
    Z_Heap *heap;
    Z_Hash_Table *table;
} Parse_Desktop_File_State;

bool proccess_desktop_file(Parse_Desktop_File_State *state, const char *pathname)
{
    Z_Heap_Auto scratch = {0};
    FILE *fp = fopen(pathname, "r");

    if (fp == NULL) {
        return false;
    }

    Z_String line = z_str_new(&scratch, "");
    Z_String_View name = z_sv("");
    Z_String_View exec = z_sv("");

    while(z_file_read_line(fp, &line) && (name.length == 0 || exec.length == 0)) {
        Z_String_View line_sv = z_sv(line);

        if (z_sv_like(line_sv, z_sv("Name=%"))) {
            name = z_sv_split_part(line_sv, z_sv("="), 1);
        } else if (z_sv_like(line_sv, z_sv("Exec=%"))) {
            exec = z_sv_split_part(line_sv, z_sv("="), 1);
        }

        z_str_clear(&line);
    }

    if (name.length > 0 && exec.length > 0) {
        z_hash_table_put(state->table, z_sv_to_cstr(state->heap, name), z_sv_to_cstr(state->heap, exec), NULL);
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
        perror("Cant open dir");
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
    Z_Heap_Auto scratch = {0};
    Z_String full_path = z_str_new(&scratch, "");

    for (const char **dir = DESKTOP_FILES_DIRECTORIES; *dir != NULL; dir++) {
        z_expand_tilde(z_sv(*dir), &full_path);
        proccess_directory(state, z_str_to_cstr(full_path));
        z_str_clear(&full_path);
    }
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

    for (size_t i = 0; i < pairs.length; i++) {
        printf("%s: %s\n", (char*)pairs.ptr[i].key, (char*)pairs.ptr[i].key);
    }

    return 0;
}
