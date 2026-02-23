#ifndef DILITHIUM_PARAMS_H
#define DILITHIUM_PARAMS_H

/*
 * ML-DSA (CRYSTALS-Dilithium) Parameter Definitions
 * Standard : NIST FIPS 204 – Module-Lattice-Based Digital Signature Algorithm
 * Author   : Jorge Ramón Figueroa Maya  –  PAP II, Week 5
 *
 * ── Protocol selection rationale ──────────────────────────────────────────
 * ML-DSA is selected over FALCON (NIST FIPS 206) and SLH-DSA (NIST FIPS 205)
 * for the following reasons:
 *
 *   1. SPEED  – Among the three NIST PQC signature finalists, Dilithium has
 *               the fastest sign+verify cycle on general-purpose CPUs.
 *   2. SIMPLICITY – No floating-point arithmetic (unlike FALCON's NTRU-based
 *               Gaussian sampling), making constant-time implementation easier.
 *   3. LIBRARY FIT – Operates over the polynomial ring Z_q[X]/(X^N+1) using
 *               modular integer arithmetic, which maps directly to the
 *               existing big-integer primitives in quantumbib.
 *   4. STANDARDISATION – Fully standardised as NIST FIPS 204 (August 2024).
 *
 * ── Ring definition ───────────────────────────────────────────────────────
 *   R_q = Z_q[X] / (X^N + 1)
 *
 *   where:
 *     N = 256   (degree of the cyclotomic polynomial; must be a power of 2
 *                so that the NTT can be applied)
 *     q = 8 380 417 = 2^23 - 2^13 + 1   (NTT-friendly 23-bit prime;
 *                satisfies q ≡ 1 mod 2N, enabling the 512th root of unity)
 *
 * ── Parameter set summary ─────────────────────────────────────────────────
 *   Set        k   l   eta  tau  gamma1   gamma2   NIST level
 *   ML-DSA-44  4   4    2    39   2^17    (q-1)/88     2
 *   ML-DSA-65  6   5    4    49   2^19    (q-1)/32     3   ← recommended
 *   ML-DSA-87  8   7    2    60   2^19    (q-1)/32     5
 */

/* ── Shared ring parameters ───────────────────────────────────────────── */

#define DILITHIUM_N      256        /* Degree of X^N + 1; also NTT length      */
#define DILITHIUM_Q      8380417    /* Prime modulus q = 2^23 - 2^13 + 1       */
#define DILITHIUM_D      13         /* Bits dropped from t (public-key rounding) */

/* ── Montgomery arithmetic constants for q ───────────────────────────────
 *   R        = 2^32                 (Montgomery radix)
 *   MONT_R   = 2^32 mod q = 4193792
 *   QINV     = q^{-1} mod 2^32 = 58728449
 *
 *   Verification (manual):
 *     8 380 417 * 58 728 449 mod 2^32 = 1  ✓
 */
#define DILITHIUM_MONT_R  4193792U   /* 2^32 mod q                              */
#define DILITHIUM_QINV   58728449U   /* q^{-1} mod 2^32                         */

/* ── ML-DSA-44  (NIST Security Category 2 / ~AES-128 classical security) ── */
#define DILITHIUM2_K        4
#define DILITHIUM2_L        4
#define DILITHIUM2_ETA      2
#define DILITHIUM2_TAU      39
#define DILITHIUM2_BETA     78          /* beta = tau * eta                      */
#define DILITHIUM2_GAMMA1   (1 << 17)   /* 131072                                */
#define DILITHIUM2_GAMMA2   95232        /* (q - 1) / 88                          */
#define DILITHIUM2_OMEGA    80

/* ── ML-DSA-65  (NIST Security Category 3 / ~AES-192 classical security)
 *   ★ RECOMMENDED for this project: balanced security / performance.        */
#define DILITHIUM3_K        6
#define DILITHIUM3_L        5
#define DILITHIUM3_ETA      4
#define DILITHIUM3_TAU      49
#define DILITHIUM3_BETA     196         /* beta = tau * eta                      */
#define DILITHIUM3_GAMMA1   (1 << 19)   /* 524288                                */
#define DILITHIUM3_GAMMA2   261888       /* (q - 1) / 32                          */
#define DILITHIUM3_OMEGA    55

/* ── ML-DSA-87  (NIST Security Category 5 / ~AES-256 classical security) ── */
#define DILITHIUM5_K        8
#define DILITHIUM5_L        7
#define DILITHIUM5_ETA      2
#define DILITHIUM5_TAU      60
#define DILITHIUM5_BETA     120         /* beta = tau * eta                      */
#define DILITHIUM5_GAMMA1   (1 << 19)   /* 524288                                */
#define DILITHIUM5_GAMMA2   261888       /* (q - 1) / 32                          */
#define DILITHIUM5_OMEGA    75

#endif /* DILITHIUM_PARAMS_H */
