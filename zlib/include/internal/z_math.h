#ifndef Z_MATH
#define Z_MATH

static inline int z__max_int(int a, int b)
{
    return a > b ? a : b;
}

static inline int z__min_int(int a, int b)
{
    return a < b ? a : b;
}

static inline size_t z__max_size_t(size_t a, size_t b)
{
    return a > b ? a : b;
}

static inline size_t z__min_size_t(size_t a, size_t b)
{
    return a < b ? a : b;
}

static inline float z__max_float(float a, float b)
{
    return a > b ? a : b;
}

static inline float z__min_float(float a, float b)
{
    return a < b ? a : b;
}

static inline double z__max_double(double a, double b)
{
    return a > b ? a : b;
}

static inline double z__min_double(double a, double b)
{
    return a < b ? a : b;
}

#endif
