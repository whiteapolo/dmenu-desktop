#include "./zlib/include/z_file.h"
#include "./zlib/include/z_string.h"
#include "./zlib/include/z_hash_table.h"
#include "./zlib/include/z_heap.h"
#include "zlib/include/z_scanner.h"
#include <stdio.h>

const char *DESKTOP_FILES_SEARCH_PATH[] = {
        "/usr/share/applications",
        "~/.local/share/applications",
        "/var/lib/flatpak/exports/share/applications",
        NULL,
};

typedef struct {
        bool ok;
        Z_String_View name;
        Z_String_View exec;
} Parse_Desktop_File_Result;

Parse_Desktop_File_Result parse_desktop_file(Z_String_View content)
{
        Z_Heap_Auto heap = {0};
        Z_String_View_Array lines = z_sv_split(&heap, content, z_sv("\n"));
        Z_String_View name = z_sv("");
        Z_String_View exec = z_sv("");

        Z_Scanner scanner = z_scanner_new(content);

        // while (!z_scanner_is_at_end(&scanner)) {
        //         z_scanner_advance_until(&scanner, '\n');
        //         Z_String_View line = z_scanner_capture(&scanner);

        //         if (z_sv_like(line, z_sv("Name=%"))) {
        //                 name = z_sv_advance(line, 5);
        //         } else if (z_sv_like(line, z_sv("Exec=%"))) {
        //                 exec = z_sv_advance(line, 5);
        //         }

        //         z_scanner_advance(&scanner, 1);
        //         z_scanner_reset_mark(&scanner);
        // }

        for (size_t i = 0; i < lines.length; i++) {
                if (z_sv_like(lines.ptr[i], z_sv("Name=%"))) {
                        name = z_sv_advance(lines.ptr[i], 5);
                } else if (z_sv_like(lines.ptr[i], z_sv("Exec=%"))) {
                        exec = z_sv_advance(lines.ptr[i], 5);
                }
        }

        if (name.length > 0 && exec.length > 0) {
                Parse_Desktop_File_Result result = { .ok = true, .name = name, .exec = exec };
                return result;
        }

        Parse_Desktop_File_Result result = { .ok = false };

        return result;
}

void proccess_desktop_file(Z_Heap *heap, const char *pathname, Z_Hash_Table *table)
{
        Z_Heap_Auto scratch = {0};
        Z_Maybe_String result =  z_read_file(&scratch, pathname);

        if (!result.ok) {
                return;
        }

        Parse_Desktop_File_Result parse_result = parse_desktop_file(z_sv(result.value));

        if (!parse_result.ok) {
                return;
        }

        z_hash_table_put(table, z_sv_to_cstr(heap, parse_result.name), z_sv_to_cstr(heap, parse_result.exec));
}

bool is_desktop_file(Z_String_View pathname)
{
        bool is_hidden = z_sv_starts_with(pathname, z_sv("."));
        bool is_desktop_extension = z_sv_ends_with(pathname, z_sv(".desktop"));

        return !is_hidden && is_desktop_extension;
}

void proccess_directory(Z_Heap *heap, const char *pathname, Z_Hash_Table *table)
{
        Z_Heap_Auto scratch = {0};
        Z_Maybe_String_Array result = z_read_directory(&scratch, pathname);

        if (!result.ok) {
                return;
        }

        Z_String_Array files = result.value;
        Z_String full_path = z_str_new(&scratch, "");

        for (size_t i = 0; i < files.length; i++) {
                if (is_desktop_file(z_sv(files.ptr[i]))) {
                        z_str_append_format(&full_path, "%s/%s", pathname, files.ptr[i].ptr);
                        proccess_desktop_file(heap, full_path.ptr, table);
                        z_str_clear(&full_path);
                }
        }
}

void proccess_directories(Z_Heap *heap, Z_Hash_Table *table)
{
        for (const char **curr = DESKTOP_FILES_SEARCH_PATH; *curr; curr++) {
                proccess_directory(heap, *curr, table);
        }
}


int main()
{
        Z_Heap_Auto heap = {0};
        Z_Hash_Table table = z_hash_table_new(&heap, z_str_equal, z_str_hash);
        proccess_directories(&heap, &table);
        return;

        Z_Pair_Array pairs = z_hash_table_to_array(&heap, &table);

        for (size_t i = 0; i < pairs.length; i++) {
                printf("\t'%s': '%s'\n", (char*)pairs.ptr[i].key, (char*)pairs.ptr[i].key);
        }

        return 0;
}
