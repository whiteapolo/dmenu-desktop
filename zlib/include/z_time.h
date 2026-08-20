#ifndef TIME_H
#define TIME_H

#include <time.h>

typedef clock_t Z_Clock;

clock_t z_get_clock(void);

double z_clock_get_elapsed_seconds(Z_Clock start);
double z_clock_get_elapsed_mseconds(Z_Clock start);

void z_print_elapsed_seconds(clock_t start);
void z_print_elapsed_mseconds(Z_Clock start);

#endif
