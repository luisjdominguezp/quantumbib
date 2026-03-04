#ifndef DILITHIUM_POLY_H
#define DILITHIUM_POLY_H

/*
 * Polynomial types and arithmetic for ML-DSA (CRYSTALS-Dilithium)
 * Standard : NIST FIPS 204
 * Author   : Jorge Ramón Figueroa Maya  –  PAP II, Week 6
 *
 * The ring is R_q = Z_q[X] / (X^N + 1),  N = 256,  q = 8 380 417.
 * Polynomials are arrays of N int32_t coefficients.
 *
 * Coefficient invariants (by function):
 *   – After poly_reduce  : coeffs ∈ (-q, q)
 *   – After poly_caddq   : coeffs ∈ [0, q)
 *   – After poly_uniform : coeffs ∈ [0, q)
 *   – After poly_uniform_eta : coeffs ∈ [-eta, eta]
 */

#include <stdint.h>
#include <stddef.h>
#include "dilithium_params.h"
#include "dilithium_reduce.h"

/* ── Types ─────────────────────────────────────────────────────────────── */

typedef struct {
    int32_t coeffs[DILITHIUM_N];
} poly;

/* Worst-case dimensions: ML-DSA-87 has k=8, l=7 */
#define DILITHIUM_K_MAX  8
#define DILITHIUM_L_MAX  7

typedef struct { poly vec[DILITHIUM_K_MAX]; } polyveck;  /* k-length vector */
typedef struct { poly vec[DILITHIUM_L_MAX]; } polyvecl;  /* l-length vector */
typedef struct { poly mat[DILITHIUM_K_MAX][DILITHIUM_L_MAX]; } polymat; /* k×l */

/* ── Polynomial arithmetic ─────────────────────────────────────────────── */

/* c = a + b  (coefficientwise) */
void poly_add(poly *c, const poly *a, const poly *b);

/* c = a - b  (coefficientwise) */
void poly_sub(poly *c, const poly *a, const poly *b);

/* Reduce all coefficients into (-q, q) via Barrett */
void poly_reduce(poly *a);

/* Normalise all coefficients into [0, q) */
void poly_caddq(poly *a);

/*
 * poly_schoolbook_mul – c = a · b  mod (X^N + 1, q)
 *
 * Schoolbook O(N²) convolution.  Uses X^N ≡ -1 to fold the upper half.
 * Intermediate accumulation in int64_t to prevent overflow (max |term| < q²).
 */
void poly_schoolbook_mul(poly *c, const poly *a, const poly *b);

/* Infinity norm: max |a->coeffs[i]| */
int32_t poly_inf_norm(const poly *a);

/* ── Decomposition helpers ─────────────────────────────────────────────── */

/*
 * poly_power2round – Lossless decomposition of t into (t1, t0).
 *
 * For each coefficient r ∈ [0, q):
 *   t0[i] = r  mod± 2^d   (centred, ∈ (-2^{d-1}, 2^{d-1}])
 *   t1[i] = (r - t0[i]) / 2^d
 * where d = DILITHIUM_D = 13.
 */
void poly_power2round(poly *t1, poly *t0, const poly *t);

/*
 * poly_highbits / poly_lowbits – Decompose  r = r1·α + r0 mod q
 *
 * α = 2·gamma2.  r0 is centred in (-gamma2, gamma2], r1 = (r-r0)/α.
 * Edge case: r1 = (q-1)/α → r1 = 0, r0 = r0 - 1.
 */
void poly_highbits(poly *r1, const poly *r, int32_t gamma2);
void poly_lowbits (poly *r0, const poly *r, int32_t gamma2);

/* poly_makehint – hint[i] = 1 iff HighBits(r+z, gamma2) ≠ HighBits(r, gamma2) */
int poly_makehint(poly *h, const poly *z, const poly *r, int32_t gamma2);

/* poly_usehint   – recover w1 from (hint, r) */
void poly_usehint(poly *w1, const poly *h, const poly *r, int32_t gamma2);

/* ── Polynomial sampling ───────────────────────────────────────────────── */

/*
 * poly_uniform – Sample uniform polynomial in [0, q) via SHAKE-128.
 *
 * Algorithm 30, FIPS 204 (RejNTTPoly).
 * Input: 32-byte seed rho, 16-bit nonce (encodes matrix (row<<8)|col).
 */
void poly_uniform(poly *a, const uint8_t seed[32], uint16_t nonce);

/*
 * poly_uniform_eta – Sample small polynomial with coefficients in [-eta, eta].
 *
 * Algorithm 31, FIPS 204 (RejBoundedPoly).
 * Input: 64-byte seed rho_prime, 16-bit nonce, eta ∈ {2, 4}.
 */
void poly_uniform_eta(poly *a, const uint8_t seed[64], uint16_t nonce, int eta);

/*
 * poly_uniform_gamma1 – Sample masking polynomial with coefficients in
 *   (-gamma1, gamma1].
 *
 * Algorithm 32, FIPS 204.
 * Input: 64-byte seed rho_prime, 16-bit nonce, gamma1 ∈ {2^17, 2^19}.
 */
void poly_uniform_gamma1(poly *a, const uint8_t seed[64],
                         uint16_t nonce, int32_t gamma1);

/*
 * poly_challenge – Sample sparse ±1 challenge polynomial (SampleInBall).
 *
 * Algorithm 33, FIPS 204.
 * Input: 32-byte hash seed, tau = number of ±1 entries.
 * Output: polynomial with exactly tau non-zero entries, each ±1.
 */
void poly_challenge(poly *c, const uint8_t seed[32], int tau);

/* ── Matrix / vector operations ────────────────────────────────────────── */

/*
 * expand_matrix – Expand seed rho into a k×l public matrix A.
 *
 * A[i][j] = poly_uniform(rho, (i<<8)|j),  coefficients in [0, q).
 */
void expand_matrix(polymat *A, const uint8_t rho[32], int k, int l);

/*
 * polyvecl_matrix_mul – t = A · v   (k-vector output, l-vector input)
 *
 * t[i] = Σ_{j=0}^{l-1}  A[i][j] · v[j]   (schoolbook poly mul)
 * Output coefficients are reduced into (-q, q) per poly_reduce.
 */
void polyvecl_matrix_mul(polyveck *t, const polymat *A,
                         const polyvecl *v, int k, int l);

/* poly_vec_add – c[i] = a[i] + b[i]  for n polynomials */
void poly_vec_add(poly *c, const poly *a, const poly *b, int n);

/* poly_vec_sub – c[i] = a[i] - b[i]  for n polynomials */
void poly_vec_sub(poly *c, const poly *a, const poly *b, int n);

/* poly_vec_mul_scalar – c[i] = scalar · v[i]  (poly multiplication) */
void poly_vec_mul_scalar(poly *c, const poly *scalar,
                         const poly *v, int n);

/* poly_vec_reduce – Barrett-reduce all polynomials in a length-n vector */
void poly_vec_reduce(poly *v, int n);

/* poly_vec_caddq – normalise to [0,q) all polynomials in a length-n vector */
void poly_vec_caddq(poly *v, int n);

/* poly_vec_inf_norm – max infinity norm across a length-n vector */
int32_t poly_vec_inf_norm(const poly *v, int n);

#endif /* DILITHIUM_POLY_H */
