#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <z_heap.h>
#include <z_string.h>
#include <dirent.h>

typedef enum {
    Z_Redirect_Stdin = 0b001,
    Z_Redirect_Stdout = 0b010,
    Z_Redirect_Stderr = 0b100,
} Z_Redirect;

typedef struct {
    FILE *stdin;
    FILE *stdout;
    FILE *stderr;
} Z_Piped_Process;

#define Z_File_Auto __attribute__((cleanup(z__file_auto_cleanup))) FILE
#define Z_Dir_Auto __attribute__((cleanup(z__dir_auto_cleanup))) DIR

void z__file_auto_cleanup(FILE **fp);
void z__dir_auto_cleanup(DIR **dir);

bool z_file_write(const char *pathname, const char *format, ...);
bool z_file_append(const char *pathname, const char *format, ...);
bool z_file_scanf(const char *pathname, const char *format, ...);

size_t z_file_read_line(FILE *fp, Z_String *out);
Z_Piped_Process z_pipe_process(char *args[], Z_Redirect redirect);

#endif
