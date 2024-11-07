#ifndef RANDOM_H
#define RANDOM_H

#include <stdio.h>
#include <gmp.h>

void q_random(unsigned long long p[], gmp_randstate_t state, int size);

#endif // RANDOM_H
