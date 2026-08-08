#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <z_heap.h>
#include <z_string.h>

bool z_file_write(const char *pathname, const char *format, ...);
bool z_file_append(const char *pathname, const char *format, ...);
bool z_file_scanf(const char *pathname, const char *format, ...);

size_t z_file_read_line(FILE *fp, Z_String *out);

// TODO: fix
// Z_Maybe_String_Array z_read_directory(Z_Heap *heap, const char *pathname);

#endif