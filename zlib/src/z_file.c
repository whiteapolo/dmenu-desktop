#include <stdio.h>
#include <z_file.h>
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
  FILE *fp = fopen(pathname, "r");

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
  DIR *directory = opendir(pathname);

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

