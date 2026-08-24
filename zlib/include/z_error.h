#ifndef Z_ERROR_H
#define Z_ERROR_H

#include <stdarg.h>

void z_perror_format(const char *format, ...);
void z_die(const char *format, ...);

#endif