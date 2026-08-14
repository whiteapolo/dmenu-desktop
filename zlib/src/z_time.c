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

void z_print_elapsed_seconds(Z_Clock start)
{
    double elapsed_seconds = z_clock_get_elapsed_seconds(start);
    printf("%lf\n", elapsed_seconds);
}
