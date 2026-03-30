#ifndef DILITHIUM_CODEC_H
#define DILITHIUM_CODEC_H

#include <stdint.h>
#include <stddef.h>
#include "dilithium_keygen.h"
#include "dilithium_sign.h"

/* bytes empacados por tipo de polinomio */
#define POLYT1_PACKEDBYTES   320  /* 10 bits/coef */
#define POLYETA_PACKEDBYTES   96  /*  3 bits/coef, eta=2 */
#define POLYT0_PACKEDBYTES   416  /* 13 bits/coef */
#define POLYZ_PACKEDBYTES    576  /* 18 bits/coef, gamma1=2^17 */
#define POLYW1_PACKEDBYTES   192  /*  6 bits/coef, w1 in [0,43] */

/* tamaños de wire ML-DSA-44 (FIPS 204 Tabla 2) */
#define MLDSA44_PK_BYTES   1312
#define MLDSA44_SK_BYTES   2560
#define MLDSA44_SIG_BYTES  2420

void pk_encode(uint8_t pk[MLDSA44_PK_BYTES],
               const uint8_t rho[DILITHIUM_SEEDBYTES],
               const polyveck *t1);

void pk_decode(uint8_t rho[DILITHIUM_SEEDBYTES],
               polyveck *t1,
               const uint8_t pk[MLDSA44_PK_BYTES]);

void sk_encode(uint8_t sk[MLDSA44_SK_BYTES],
               const uint8_t rho[DILITHIUM_SEEDBYTES],
               const uint8_t K[DILITHIUM_SEEDBYTES],
               const uint8_t tr[DILITHIUM_TRBYTES],
               const polyvecl *s1,
               const polyveck *s2,
               const polyveck *t0);

void sk_decode(uint8_t rho[DILITHIUM_SEEDBYTES],
               uint8_t K[DILITHIUM_SEEDBYTES],
               uint8_t tr[DILITHIUM_TRBYTES],
               polyvecl *s1,
               polyveck *s2,
               polyveck *t0,
               const uint8_t sk[MLDSA44_SK_BYTES]);

void sig_encode(uint8_t sig[MLDSA44_SIG_BYTES],
                const dilithium_sig *s);

/* devuelve -1 si los hints estan mal formados */
int  sig_decode(dilithium_sig *s,
                const uint8_t sig[MLDSA44_SIG_BYTES]);

/* w1Encode — Alg. 28 FIPS 204: 6 bits/coef, k*192 bytes en total */
void w1_encode(uint8_t *out, const polyveck *w1);

#endif
