#ifndef RANDOM_H
#define RANDOM_H

#include <stdio.h>
#include <gmp.h>

void q_random(unsigned long long p[], gmp_randstate_t state, size_t size);

#endif // RANDOM_H
