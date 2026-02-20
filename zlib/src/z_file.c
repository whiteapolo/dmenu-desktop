#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <z_file.h>
#include <z_path.h>
#include <string.h>
#include <dirent.h>

size_t z__get_file_size(FILE *fp)
{
        size_t curr = ftell(fp);
        fseek(fp, 0, SEEK_END);

        size_t size = ftell(fp);
        fseek(fp, curr, SEEK_SET);

        return size;
}

bool z_write_file(const char *pathname, const char *format, ...)
{
        FILE *fp = fopen(pathname, "w");

        if (fp == NULL) {
                return false;
        }

        va_list args;
        va_start(args, format);
        vfprintf(fp, format, args);
        va_end(args);

        return true;
}

bool z_append_file(const char *pathname, const char *format, ...)
{
        FILE *fp = fopen(pathname, "a");

        if (fp == NULL) {
                return false;
        }

        va_list args;
        va_start(args, format);
        vfprintf(fp, format, args);
        va_end(args);

        return true;
}

bool z_scanf_file(const char *pathname, const char *format, ...)
{
        FILE *fp = fopen(pathname, "r");

        if (fp == NULL) {
                return false;
        }

        va_list args;
        va_start(args, format);

        if (vfscanf(fp, format, args) == EOF) {
                va_end(args);
                fclose(fp);
                return false;
        }

        va_end(args);
        fclose(fp);

        return true;
}

Z_Maybe_String z_read_file(Z_Heap *heap, const char *pathname)
{
        Z_Heap_Auto scratch = {0};
        Z_String expanded_path = z_expand_tilde(&scratch, z_sv(pathname));

        FILE *fp = fopen(expanded_path.ptr, "r");

        if (fp == NULL) {
                return (Z_Maybe_String){ .ok = false };
        }

        Z_String content = z_str_new(heap, "");
        size_t file_size = z__get_file_size(fp);

        z_array_ensure_capacity(&content, file_size);
        content.length = fread(content.ptr, sizeof(char), file_size, fp);
        z_array_zero_terminate(&content);
        fclose(fp);

        return (Z_Maybe_String){ .ok = true, .value = content };
}

Z_Maybe_String_Array z_read_directory(Z_Heap *heap, const char *pathname)
{
        Z_Heap_Auto scratch = {0};
        Z_String expanded_path = z_expand_tilde(&scratch, z_sv(pathname));

        DIR *directory = opendir(expanded_path.ptr);

        if (directory == NULL) {
                return (Z_Maybe_String_Array){ .ok = false };
        }

        Z_String_Array entries = z_array_new(heap, Z_String_Array);
        struct dirent *directory_entry;

        while ((directory_entry = readdir(directory))) {
                z_array_push(&entries, z_str_new(heap, "%s", directory_entry->d_name));
        }

        closedir(directory);

        return (Z_Maybe_String_Array){ .ok = true, .value = entries };
}

typedef enum {
        Z_REDIRECT_STDOUT = 0b1,
        Z_REDIRECT_STDIN = 0b10,
        Z_REDIRECT_STDERR = 0b100,
} Z_Redirect;

typedef struct {
        bool ok;
        FILE *stdin;
        FILE *stdout;
        FILE *stderr;
} Z_Piped_Proccess;

Z_Piped_Proccess z_piped_proccess_ok(FILE *stdin, FILE *stdout, FILE *stderr)
{
        Z_Piped_Proccess proccess = {
                .ok = true,
                .stdin = stdin,
                .stdout = stdout,
                .stderr = stderr,
        };

        return proccess;
}

Z_Piped_Proccess z_piped_proccess_error()
{
        Z_Piped_Proccess proccess = {
                .ok = false,
        };

        return proccess;
}

void z_safe_pipe(int fd[2])
{
        if (pipe(fd) == -1) {
                abort();
        }
}

int z_safe_fork()
{
        int pid = fork();

        if (pid == -1) {
                abort();
        }

        return pid;
}

Z_Piped_Proccess z_pipe_proccess(const char *pathname, char *argv[], Z_Redirect redirect)
{
        int stdout_pipe[2];
        if (redirect & Z_REDIRECT_STDOUT) {
                z_safe_pipe(stdout_pipe);
        }

        int pid = z_safe_fork();

        if (!pid) {
                // child
                dup2(stdout_pipe[1], STDOUT_FILENO);
                execvp(pathname, argv);
                abort();
        }

        // parent

        FILE *stdout = fdopen(stdout_pipe[0], "r");

        return z_piped_proccess_ok(stdout, NULL, NULL);
}

// bool z_popen2(char *pathname, char *argv[], FILE *ppipe[2]) {
//         int output[2];
//         int input[2];

//         if (pipe(output) == -1 || pipe(input) == -1) {
//                 return false;
//         }

//         int pid = fork();

//         if (pid == -1) {
//                 return false;
//         }

//         if (pid) {
//                 // parent
//                 close(output[Z_Pipe_Mode_Write]);
//                 ppipe[Z_Pipe_Mode_Write] = fdopen(input[Z_Pipe_Mode_Write], "w");
//                 ppipe[Z_Pipe_Mode_Read] = fdopen(output[Z_Pipe_Mode_Read], "r");
//         } else {
//                 // child
//                 dup2(input[Z_Pipe_Mode_Read], STDIN_FILENO);
//                 dup2(output[Z_Pipe_Mode_Write], STDOUT_FILENO);
//                 close(input[Z_Pipe_Mode_Write]);
//                 close(input[Z_Pipe_Mode_Read]);
//                 close(output[Z_Pipe_Mode_Write]);
//                 close(output[Z_Pipe_Mode_Read]);
//                 execvp(pathname, argv);
//                 exit(EXIT_FAILURE);
//         }

//         return true;
// }
