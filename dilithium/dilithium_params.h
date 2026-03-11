#ifndef DILITHIUM_PARAMS_H
#define DILITHIUM_PARAMS_H

/*
 * Dilithium / ML-DSA parameter definitions.
 * See https://github.com/pq-crystals/dilithium (ref/params.h)
 *
 * All symbols prefixed DILITHIUM_ to avoid collisions with quantumbib.
 */

#include <stdint.h>

/* Seed / hash sizes (same across all parameter sets) */
#define DILITHIUM_SEEDBYTES  32
#define DILITHIUM_CRHBYTES   64
#define DILITHIUM_TRBYTES    64
#define DILITHIUM_RNDBYTES   32

/* Shared ring parameters */
#define DILITHIUM_N          256     /* Polynomial degree                    */
#define DILITHIUM_Q          8380417 /* Prime modulus  q = 2^23 - 2^13 + 1  */
#define DILITHIUM_D          13      /* Dropped bits in Power2Round          */
#define DILITHIUM_ROOT_OF_UNITY 1753

/* Montgomery constants */
#define DILITHIUM_MONT      (-4186625)  /* 2^32 % Q  (signed representation) */
#define DILITHIUM_QINV      58728449    /* q^(-1) mod 2^32                   */

/* ML-DSA-44 (NIST level 2) */
#define DILITHIUM2_K         4
#define DILITHIUM2_L         4
#define DILITHIUM2_ETA       2
#define DILITHIUM2_TAU       39
#define DILITHIUM2_BETA      78       /* tau * eta                          */
#define DILITHIUM2_GAMMA1    (1 << 17)
#define DILITHIUM2_GAMMA2    ((DILITHIUM_Q - 1) / 88)  /* 95232            */
#define DILITHIUM2_OMEGA     80
#define DILITHIUM2_CTILDEBYTES 32

/* ML-DSA-65 (NIST level 3) */
#define DILITHIUM3_K         6
#define DILITHIUM3_L         5
#define DILITHIUM3_ETA       4
#define DILITHIUM3_TAU       49
#define DILITHIUM3_BETA      196      /* tau * eta                          */
#define DILITHIUM3_GAMMA1    (1 << 19)
#define DILITHIUM3_GAMMA2    ((DILITHIUM_Q - 1) / 32)  /* 261888           */
#define DILITHIUM3_OMEGA     55
#define DILITHIUM3_CTILDEBYTES 48

/* ML-DSA-87 (NIST level 5) */
#define DILITHIUM5_K         8
#define DILITHIUM5_L         7
#define DILITHIUM5_ETA       2
#define DILITHIUM5_TAU       60
#define DILITHIUM5_BETA      120      /* tau * eta                          */
#define DILITHIUM5_GAMMA1    (1 << 19)
#define DILITHIUM5_GAMMA2    ((DILITHIUM_Q - 1) / 32)  /* 261888           */
#define DILITHIUM5_OMEGA     75
#define DILITHIUM5_CTILDEBYTES 64

#endif /* DILITHIUM_PARAMS_H */
