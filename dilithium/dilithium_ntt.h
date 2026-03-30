#ifndef DILITHIUM_NTT_H
#define DILITHIUM_NTT_H

#include <stdint.h>
#include "dilithium_poly.h"

/* tabla de raices de unidad precalculadas (forma Montgomery) */
extern const int32_t dilithium_zetas[256];

void ntt_forward(int32_t a[256]);
void ntt_inverse(int32_t a[256]);

/* multiplicacion polinomial via NTT: c = a*b mod (X^N+1, q) */
void poly_ntt_mul(poly *c, const poly *a, const poly *b);

#endif
