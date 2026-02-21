#include "./zlib/include/z_file.h"
#include "./zlib/include/z_string.h"
#include "./zlib/include/z_hash_table.h"
#include "./zlib/include/z_heap.h"
#include "zlib/include/z_path.h"
#include "zlib/include/z_scanner.h"
#include <stdio.h>

typedef struct {
    bool ok;
    Z_String_View name;
    Z_String_View exec;
} Parse_Desktop_File_Result;

Parse_Desktop_File_Result parse_desktop_file_result_error()
{
    Parse_Desktop_File_Result result = {
        .ok = false,
    };

    return result;
}

Parse_Desktop_File_Result parse_desktop_file_result_ok(Z_String_View name, Z_String_View exec)
{
    Parse_Desktop_File_Result result = {
        .ok = true,
        .name = name,
        .exec = exec,
    };

    return result;
}

Parse_Desktop_File_Result parse_desktop_file(Z_String_View content)
{
    Z_Heap_Auto heap = {0};
    Z_String_View_Array lines = z_sv_split(&heap, content, z_sv("\n"));
    Z_String_View name = z_sv("");
    Z_String_View exec = z_sv("");

    for (size_t i = 0; i < lines.length; i++) {
        if (z_sv_like(lines.ptr[i], z_sv("Name=%"))) {
            name = z_sv_split_part(lines.ptr[i], z_sv("="), 1);
        } else if (z_sv_like(lines.ptr[i], z_sv("Exec=%"))) {
            exec = z_sv_split_part(lines.ptr[i], z_sv("="), 1);
        }
    }

    if (name.length > 0 && exec.length > 0) {
        return parse_desktop_file_result_ok(name, exec);
    }

    return parse_desktop_file_result_error();
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

Z_String_Array map_directories_to_files(Z_Heap *heap, Z_String_Array directories)
{
    Z_String_Array files = z_array_new(heap, Z_String_Array);
    Z_Heap_Auto scratch = {0};

    for (size_t i = 0; i < directories.length; i++) {
        Z_Maybe_String_Array result = z_read_directory(&scratch, directories.ptr[i].ptr);

        if (result.ok) {
            Z_String_Array files_in_dir = result.value;
            z_array_map(&files_in_dir, Z_String file, z_str_new(heap, "%s/%s", directories.ptr[i].ptr, file.ptr));
            z_array_push_array(&files, &files_in_dir);
        }

        z_heap_reset(&scratch);
    }

    return files;
}

bool is_desktop_file(Z_String_View path)
{
    return z_sv_ends_with(path, z_sv(".desktop"));
}

Z_String_Array get_desktop_files(Z_Heap *heap)
{
    const char *DESKTOP_FILES_SEARCH_PATH[] = {
        "/usr/share/applications",
        "~/.local/share/applications",
        "/var/lib/flatpak/exports/share/applications",
        NULL,
    };

    Z_String_Array paths = z_str_array_from(heap, DESKTOP_FILES_SEARCH_PATH);
    Z_String_Array files = map_directories_to_files(heap, paths);
    z_array_filter(&files, Z_String file, is_desktop_file(z_sv(file)));

    return files;
}

int main()
<%
    Z_Heap_Auto heap = {0};
    Z_Hash_Table table = z_hash_table_new(&heap, z_str_equal, z_str_hash);
    Z_String_Array desktop_files = get_desktop_files(&heap);

    for (size_t i = 0; i < desktop_files.length; i++) {
        proccess_desktop_file(&heap, desktop_files.ptr[i].ptr, &table);
    }

    Z_Pair_Array pairs = z_hash_table_to_array(&heap, &table);

    for (size_t i = 0; i < pairs.length; i++) {
        printf("%s: %40s\n", (char*)pairs.ptr[i].key, (char*)pairs.ptr[i].key);
    }

    return 0;
}
