#ifndef DILITHIUM_REDUCE_H
#define DILITHIUM_REDUCE_H

/*
 * Modular reduction primitives for ML-DSA (CRYSTALS-Dilithium)
 * Standard : NIST FIPS 204
 * Author   : Jorge Ramón Figueroa Maya  –  PAP II, Week 5
 *
 * All three functions operate modulo q = 8 380 417 = 2^23 - 2^13 + 1.
 * They are independent of the generic Barrett/Montgomery implementations
 * in quantumbib because Dilithium's q fits in 23 bits, enabling faster
 * word-level arithmetic than GMP-based big-integer routines.
 */

#include <stdint.h>
#include "dilithium_params.h"

/*
 * dilithium_reduce32  –  Barrett-style reduction exploiting q ≈ 2^23
 *
 * Because q = 2^23 - 2^13 + 1 ≈ 2^23, an approximate quotient is:
 *   t = round(a / q) ≈ round(a / 2^23) = (a + 2^22) >> 23
 * The remainder a - t*q is then exact and falls in (-q, q).
 *
 * Input : a  in [-2^31, 2^31)   (any int32_t)
 * Output: r ≡ a (mod q),  r in (-q, q)
 *
 * NOTE: relies on arithmetic (signed) right-shift, which is
 * implementation-defined in C99/C11 but universally provided by
 * GCC/Clang on the x86/ARM targets used in this project (-march=native).
 */
int32_t dilithium_reduce32(int32_t a);

/*
 * dilithium_montgomery_reduce  –  Montgomery reduction mod q
 *
 * Computes a * R^{-1} mod q where R = 2^32.
 * Uses QINV = q^{-1} mod 2^32 = 58 728 449.
 *
 * Algorithm:
 *   t = (int32_t)(a mod R) * QINV   →  t * q ≡ a  (mod 2^32)
 *   return (a - t * q) >> 32        →  exact (no remainder), in (-q, q)
 *
 * Input : a  in (-q * 2^32,  q * 2^32)
 * Output: r ≡ a * 2^{-32} (mod q),  r in (-q, q)
 *
 * Reference: NIST FIPS 204, Section 2.5 / Dilithium spec §2.4
 */
int32_t dilithium_montgomery_reduce(int64_t a);

/*
 * dilithium_caddq  –  Conditional add q (branch-free normalisation)
 *
 * Maps any value in (-q, q) to the canonical representative in [0, q).
 *
 * Input : a in (-q, q)
 * Output: r in [0, q),  r ≡ a (mod q)
 */
int32_t dilithium_caddq(int32_t a);

#endif /* DILITHIUM_REDUCE_H */
