#ifndef MONTGOMERY_EXPO_H
#define MONTGOMERY_EXPO_H
#include <gmp.h>

void montgomery_e(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int size, int r_size, int bit_limit, mpz_t mod_hex);

#endif // MONTGOMERY_EXPO_H
