#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <z_heap.h>
#include <z_string.h>

typedef enum {
    Z_Redirect_Stdin = 0b001,
    Z_Redirect_Stdout = 0b010,
    Z_Redirect_Stderr = 0b100,
} Z_Redirect;

typedef struct {
    FILE *stdin;
    FILE *stdout;
    FILE *stderr;
} Z_Piped_Proccess;

bool z_file_write(const char *pathname, const char *format, ...);
bool z_file_append(const char *pathname, const char *format, ...);
bool z_file_scanf(const char *pathname, const char *format, ...);

size_t z_file_read_line(FILE *fp, Z_String *out);
Z_Piped_Proccess z_pipe_proccess(char *args[], Z_Redirect redirect);

// TODO: fix
// Z_Maybe_String_Array z_read_directory(Z_Heap *heap, const char *pathname);

#endif