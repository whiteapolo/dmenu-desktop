#include <stdio.h>
#include <z_file.h>
#include <string.h>
#include <dirent.h>

size_t z__get_file_size(FILE *fp);

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
