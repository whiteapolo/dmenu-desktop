#include <z_error.h>
#include <z_heap.h>
#include <z_string.h>
#include <stdio.h>

void z_perror_format_va(const char *format, va_list args);

void z_perror_format(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    z_perror_format_va(format, args);
    va_end(args);
}

void z_perror_format_va(const char *format, va_list args)
{
    Z_Heap_Auto heap = {0};
    Z_String s = z_str_new(&heap, "");

    z_str_append_format_va(&s, format, args);
    perror(z_str_to_cstr(s));
}
