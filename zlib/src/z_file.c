#include <stdio.h>
#include <z_file.h>
#include <string.h>
#include <dirent.h>

#define PIPE_IN 1
#define PIPE_OUT 0

FILE *fdopen(int fd, const char *mode);
size_t z__get_file_size(FILE *fp);
void z_safe_pipe(int fd[2]);
int z_safe_fork(void);

size_t z__get_file_size(FILE *fp)
{
    long curr = ftell(fp);
    assert(curr >= 0);
    fseek(fp, 0, SEEK_END);

    long size = ftell(fp);
    assert(size >= 0);
    fseek(fp, curr, SEEK_SET);

    return (size_t)size;
}

bool z_file_write(const char *pathname, const char *format, ...)
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

bool z_file_append(const char *pathname, const char *format, ...)
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

bool z_file_scanf(const char *pathname, const char *format, ...)
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

size_t z_file_read_line(FILE *fp, Z_String *out)
{
    char buffer[READ_BUFFER_SIZE] = {0};
    size_t start_length = out->length;

    while (fgets(buffer, READ_BUFFER_SIZE, fp)) {
        z_str_append_cstr(out, buffer);

        if (strchr(buffer, '\n')) {
            break;
        }
    }

    return out->length - start_length;
}

void z_safe_pipe(int fd[2])
{
    if (pipe(fd) == -1) {
        perror("pipe() faild");
        exit(EXIT_FAILURE);
    }
}

int z_safe_fork(void)
{
    int pid = fork();

    if (pid == -1) {
        perror("fork() faild");
        exit(EXIT_FAILURE);
    }

    return pid;
}

Z_Piped_Proccess z_pipe_proccess(char *args[], Z_Redirect redirect)
{
    int stdin[2];
    int stdout[2];
    int stderr[2];

    if (redirect & Z_Redirect_Stdin) z_safe_pipe(stdin);
    if (redirect & Z_Redirect_Stdout) z_safe_pipe(stdout);
    if (redirect & Z_Redirect_Stderr) z_safe_pipe(stderr);

    int pid = z_safe_fork();

    if (pid == 0) { // child
        if (redirect & Z_Redirect_Stdin) {
            dup2(stdin[PIPE_OUT], STDIN_FILENO);
            close(stdin[PIPE_IN]);
        }

        if (redirect & Z_Redirect_Stdout) {
            dup2(stdout[PIPE_IN], STDOUT_FILENO);
            close(stdout[PIPE_OUT]);
        }

        if (redirect & Z_Redirect_Stderr) {
            dup2(stderr[PIPE_IN], STDERR_FILENO);
            close(stderr[PIPE_OUT]);
        }

        execvp(args[0], args);
        perror("exec() failed");
        exit(EXIT_FAILURE);
    }

    Z_Piped_Proccess piped_process = {0};

    if (redirect & Z_Redirect_Stdin) {
        close(stdin[PIPE_OUT]);
        piped_process.stdin = fdopen(stdin[PIPE_IN], "w");
    }

    if (redirect & Z_Redirect_Stdout) {
        close(stdout[PIPE_IN]);
        piped_process.stdout = fdopen(stdout[PIPE_OUT], "r");
    }

    if (redirect & Z_Redirect_Stderr) {
        close(stderr[PIPE_IN]);
        piped_process.stderr = fdopen(stderr[PIPE_OUT], "r");
    }

    return piped_process;
}
