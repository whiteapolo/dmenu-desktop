#ifndef FILE_H
#define FILE_H

#include <z_heap.h>
#include <z_string.h>

typedef struct {
  Z_String value;
  bool ok;
} Z_Maybe_String;

typedef struct {
  Z_String_Array value;
  bool ok;
} Z_Maybe_String_Array;

bool z_write_file(const char *pathname, const char *format, ...);
bool z_append_file(const char *pathname, const char *format, ...);
bool z_scanf_file(const char *pathname, const char *format, ...);

Z_Maybe_String z_read_file(Z_Heap *heap, const char *pathname);
Z_Maybe_String_Array z_read_directory(Z_Heap *heap, const char *pathname);

#endif