#include <string.h>

int z_compare_int_pointers(const int *a, const int *b)
{
  return (*a > *b) - (*a < *b);
}

int z_compare_float_pointers(const float *a, const float *b)
{
  return (*a > *b) - (*a < *b);
}

int z_compare_double_pointers(const double *a, const double *b)
{
  return (*a > *b) - (*a < *b);
}

int z_compare_string_pointers(const char **a, const char **b)
{
  return strcmp(*a, *b);
}