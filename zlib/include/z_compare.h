#ifndef COMPARE_H
#define COMPARE_H

typedef int (*Z_Compare_Fn)(const void *, const void *);
typedef void (*Z_Print_Fn)(const void *);

int z_compare_int_pointers(const int *a, const int *b);
int z_compare_float_pointers(const float *a, const float *b);
int z_compare_double_pointers(const double *a, const double *b);
int z_compare_string_pointers(const char **a, const char **b);

#endif