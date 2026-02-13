#include <z_time.h>
#include <stdio.h>

Z_Clock z_get_clock()
{
  return clock();
}

void z_print_elapsed_seconds(Z_Clock start)
{
  double elapsed_seconds = ((double)(z_get_clock() - start)) / CLOCKS_PER_SEC;
  printf("%lf\n", elapsed_seconds);
}
