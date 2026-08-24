#include <z_time.h>
#include <stdio.h>

Z_Clock z_get_clock(void)
{
    return clock();
}

double z_clock_get_elapsed_seconds(Z_Clock start)
{
    return ((double)(z_get_clock() - start)) / CLOCKS_PER_SEC;
}

double z_clock_get_elapsed_mseconds(Z_Clock start)
{
    return z_clock_get_elapsed_seconds(start) * 1000;
}

void z_print_elapsed_seconds(Z_Clock start)
{
    double elapsed_seconds = z_clock_get_elapsed_seconds(start);
    printf("%lfs\n", elapsed_seconds);
}

void z_print_elapsed_mseconds(Z_Clock start)
{
    double elapsed_mseconds = z_clock_get_elapsed_mseconds(start);
    printf("%lfms\n", elapsed_mseconds);
}
