#include "dilithium_reduce.h"

/*
 * Modular reduction primitives for ML-DSA (CRYSTALS-Dilithium)
 * Standard : NIST FIPS 204
 * Author   : Jorge Ramón Figueroa Maya  –  PAP II, Week 5
 *
 * Reference implementation: pq-crystals/dilithium (ref/reduce.c)
 * Adapted to the quantumbib coding conventions and documented for clarity.
 */

/*
 * Barrett reduction for q = 2^23 - 2^13 + 1.
 *
 * Key insight: the special structure of q means q ≈ 2^23, so dividing by
 * q can be approximated cheaply by an arithmetic right-shift by 23 bits.
 * Adding 2^22 before the shift achieves correct rounding (round-to-nearest).
 *
 * Correctness argument:
 *   Let t = (a + 2^22) >> 23 = floor((a + 2^22) / 2^23).
 *   Then |a/q - t| < 1  =>  |a - t*q| < q.
 *   Therefore the result a - t*q lies in the open interval (-q, q).
 */
int32_t dilithium_reduce32(int32_t a) {
    int32_t t = (a + (1 << 22)) >> 23;
    return a - t * (int32_t)DILITHIUM_Q;
}

/*
 * Montgomery reduction mod q.
 *
 * Step-by-step (R = 2^32, QINV = q^{-1} mod R):
 *   1. Cast 'a' to int32_t to isolate its low 32 bits (a mod R).
 *   2. Multiply by QINV: t = (int32_t)(a * QINV mod R).
 *      By construction, t * q ≡ a (mod R), so (a - t*q) ≡ 0 (mod R).
 *   3. Compute (a - t*q) in 64-bit arithmetic, then shift right 32 bits.
 *      Because the lower 32 bits are zero the shift is exact (integer div).
 *   4. The result is in (-q, q).  Use dilithium_caddq() to lift to [0, q).
 */
int32_t dilithium_montgomery_reduce(int64_t a) {
    int32_t t = (int32_t)((int64_t)(int32_t)a * (int64_t)DILITHIUM_QINV);
    return (int32_t)((a - (int64_t)t * (int64_t)DILITHIUM_Q) >> 32);
}

/*
 * Conditional add q – branch-free normalisation to [0, q).
 *
 * (a >> 31) produces the sign mask: 0xFFFFFFFF if a < 0, else 0x00000000.
 * ANDing with q and adding it to a adds q iff a is negative.
 */
int32_t dilithium_caddq(int32_t a) {
    return a + ((a >> 31) & (int32_t)DILITHIUM_Q);
}
