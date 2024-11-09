#ifndef MONTGOMERY_H
#define MONTGOMERY_H

#include <gmp.h>

void montgomery_pr(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int size, int r_size, int bit_limit, mpz_t mod_hex);

#endif // MONTGOMERY_H
