#ifndef DILITHIUM_POLY_H
#define DILITHIUM_POLY_H

/*
 * Polynomial types, arithmetic and sampling for Dilithium.
 * Merges ref/poly.h, ref/polyvec.h and ref/rounding.h from
 * https://github.com/pq-crystals/dilithium
 *
 * Main differences vs the reference:
 *  - Vectors use fixed worst-case dimensions (K_MAX=8, L_MAX=7) so
 *    one build works for all parameter sets.
 *  - NTT multiplication replaced by schoolbook O(N^2).
 *  - Sampling functions take eta/gamma1/tau as arguments.
 */

#include <stdint.h>
#include <stddef.h>
#include "dilithium_params.h"
#include "dilithium_reduce.h"

/* Types */

typedef struct {
  int32_t coeffs[DILITHIUM_N];
} poly;

/* Worst-case dimensions: ML-DSA-87 has k=8, l=7 */
#define DILITHIUM_K_MAX  8
#define DILITHIUM_L_MAX  7

typedef struct { poly vec[DILITHIUM_K_MAX]; } polyveck;
typedef struct { poly vec[DILITHIUM_L_MAX]; } polyvecl;
typedef struct { poly mat[DILITHIUM_K_MAX][DILITHIUM_L_MAX]; } polymat;

/* Polynomial arithmetic (from ref/poly.h) */

/*************************************************
* Name:        poly_reduce
*
* Description: Inplace reduction of all coefficients of polynomial to
*              representative in [-6283008,6283008].
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
void poly_reduce(poly *a);

/*************************************************
* Name:        poly_caddq
*
* Description: For all coefficients of in/out polynomial add Q if
*              coefficient is negative.
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
void poly_caddq(poly *a);

/*************************************************
* Name:        poly_add
*
* Description: Add polynomials. No modular reduction is performed.
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const poly *a: pointer to first summand
*              - const poly *b: pointer to second summand
**************************************************/
void poly_add(poly *c, const poly *a, const poly *b);

/*************************************************
* Name:        poly_sub
*
* Description: Subtract polynomials. No modular reduction is
*              performed.
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const poly *a: pointer to first input polynomial
*              - const poly *b: pointer to second input polynomial to be
*                                subtracted from first input polynomial
**************************************************/
void poly_sub(poly *c, const poly *a, const poly *b);

/*************************************************
* Name:        poly_shiftl
*
* Description: Multiply polynomial by 2^D without modular reduction.
*              Assumes input coefficients to be less than 2^{31-D} in
*              absolute value.
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
void poly_shiftl(poly *a);

/*************************************************
* Name:        poly_schoolbook_mul
*
* Description: Schoolbook O(N^2) polynomial multiplication in
*              R_q = Z_q[X]/(X^N+1).  Used instead of NTT.
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const poly *a: pointer to first input polynomial
*              - const poly *b: pointer to second input polynomial
**************************************************/
void poly_schoolbook_mul(poly *c, const poly *a, const poly *b);

/*************************************************
* Name:        poly_chknorm
*
* Description: Check infinity norm of polynomial against given bound.
*              Assumes input coefficients were reduced by reduce32().
*
* Arguments:   - const poly *a: pointer to polynomial
*              - int32_t B: norm bound
*
* Returns 0 if norm is strictly smaller than B <= (Q-1)/8 and 1 otherwise.
**************************************************/
int poly_chknorm(const poly *a, int32_t B);

/* Rounding (from ref/rounding.h) */

/*************************************************
* Name:        power2round
*
* Description: For finite field element a, compute a0, a1 such that
*              a mod^+ Q = a1*2^D + a0 with -2^{D-1} < a0 <= 2^{D-1}.
*              Assumes a to be standard representative.
*
* Arguments:   - int32_t a: input element
*              - int32_t *a0: pointer to output element a0
*
* Returns a1.
**************************************************/
int32_t power2round(int32_t *a0, int32_t a);

/*************************************************
* Name:        decompose
*
* Description: For finite field element a, compute high and low bits
*              a0, a1 such that a mod^+ Q = a1*ALPHA + a0 with
*              -ALPHA/2 < a0 <= ALPHA/2 except if a1 = (Q-1)/ALPHA
*              where we set a1 = 0 and -ALPHA/2 <= a0 = a mod^+ Q - Q < 0.
*              Assumes a to be standard representative.
*
* Arguments:   - int32_t *a0: pointer to output element a0
*              - int32_t a: input element
*              - int32_t gamma2: the gamma2 parameter
*
* Returns a1.
**************************************************/
int32_t decompose(int32_t *a0, int32_t a, int32_t gamma2);

/*************************************************
* Name:        make_hint
*
* Description: Compute hint bit indicating whether the low bits of the
*              input element overflow into the high bits.
*
* Arguments:   - int32_t a0: low bits of input element
*              - int32_t a1: high bits of input element
*              - int32_t gamma2: the gamma2 parameter
*
* Returns 1 if overflow.
**************************************************/
unsigned int make_hint(int32_t a0, int32_t a1, int32_t gamma2);

/*************************************************
* Name:        use_hint
*
* Description: Correct high bits according to hint.
*
* Arguments:   - int32_t a: input element
*              - unsigned int hint: hint bit
*              - int32_t gamma2: the gamma2 parameter
*
* Returns corrected high bits.
**************************************************/
int32_t use_hint(int32_t a, unsigned int hint, int32_t gamma2);

/* Polynomial-level rounding wrappers */

void poly_power2round(poly *a1, poly *a0, const poly *a);
void poly_decompose(poly *a1, poly *a0, const poly *a, int32_t gamma2);
unsigned int poly_make_hint(poly *h, const poly *a0, const poly *a1,
                            int32_t gamma2);
void poly_use_hint(poly *b, const poly *a, const poly *h, int32_t gamma2);

/* Sampling (from ref/poly.h) */

/*************************************************
* Name:        poly_uniform
*
* Description: Sample polynomial with uniformly random coefficients
*              in [0,Q-1] by performing rejection sampling on the
*              output stream of SHAKE128(seed|nonce).
*
* Arguments:   - poly *a: pointer to output polynomial
*              - const uint8_t seed[]: byte array with seed of length
*                                      DILITHIUM_SEEDBYTES
*              - uint16_t nonce: 2-byte nonce
**************************************************/
void poly_uniform(poly *a, const uint8_t seed[DILITHIUM_SEEDBYTES],
                  uint16_t nonce);

/*************************************************
* Name:        poly_uniform_eta
*
* Description: Sample polynomial with uniformly random coefficients
*              in [-ETA,ETA] by performing rejection sampling on the
*              output stream from SHAKE256(seed|nonce).
*
* Arguments:   - poly *a: pointer to output polynomial
*              - const uint8_t seed[]: byte array with seed of length
*                                      DILITHIUM_CRHBYTES
*              - uint16_t nonce: 2-byte nonce
*              - int eta: bound on coefficients (2 or 4)
**************************************************/
void poly_uniform_eta(poly *a, const uint8_t seed[DILITHIUM_CRHBYTES],
                      uint16_t nonce, int eta);

/*************************************************
* Name:        poly_uniform_gamma1
*
* Description: Sample polynomial with uniformly random coefficients
*              in [-(GAMMA1-1), GAMMA1] by unpacking output stream of
*              SHAKE256(seed|nonce).
*
* Arguments:   - poly *a: pointer to output polynomial
*              - const uint8_t seed[]: byte array with seed of length
*                                      DILITHIUM_CRHBYTES
*              - uint16_t nonce: 16-bit nonce
*              - int32_t gamma1: bound on coefficients
**************************************************/
void poly_uniform_gamma1(poly *a, const uint8_t seed[DILITHIUM_CRHBYTES],
                         uint16_t nonce, int32_t gamma1);

/*************************************************
* Name:        poly_challenge
*
* Description: Implementation of H. Samples polynomial with TAU nonzero
*              coefficients in {-1,1} using the output stream of
*              SHAKE256(seed).
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const uint8_t seed[]: byte array containing seed
*              - int tau: number of +/-1 coefficients
**************************************************/
void poly_challenge(poly *c, const uint8_t *seed, int tau);

/* Vector / matrix operations (from ref/polyvec.h) */

void expand_matrix(polymat *A, const uint8_t rho[DILITHIUM_SEEDBYTES],
                   int k, int l);

void polyvecl_uniform_eta(polyvecl *v, const uint8_t seed[DILITHIUM_CRHBYTES],
                          uint16_t nonce, int l, int eta);
void polyveck_uniform_eta(polyveck *v, const uint8_t seed[DILITHIUM_CRHBYTES],
                          uint16_t nonce, int k, int eta);

void polyvecl_uniform_gamma1(polyvecl *v,
                             const uint8_t seed[DILITHIUM_CRHBYTES],
                             uint16_t nonce, int l, int32_t gamma1);

void polyvecl_reduce(polyvecl *v, int l);
void polyvecl_add(polyvecl *w, const polyvecl *u, const polyvecl *v, int l);
void polyvecl_sub(polyvecl *w, const polyvecl *u, const polyvecl *v, int l);
void polyvecl_pointwise_poly(polyvecl *r, const poly *a,
                             const polyvecl *v, int l);
int polyvecl_chknorm(const polyvecl *v, int l, int32_t bound);

void polyveck_reduce(polyveck *v, int k);
void polyveck_caddq(polyveck *v, int k);
void polyveck_add(polyveck *w, const polyveck *u, const polyveck *v, int k);
void polyveck_sub(polyveck *w, const polyveck *u, const polyveck *v, int k);
void polyveck_shiftl(polyveck *v, int k);
void polyveck_pointwise_poly(polyveck *r, const poly *a,
                             const polyveck *v, int k);
int polyveck_chknorm(const polyveck *v, int k, int32_t bound);

void polyveck_power2round(polyveck *v1, polyveck *v0, const polyveck *v,
                          int k);
void polyveck_decompose(polyveck *v1, polyveck *v0, const polyveck *v,
                        int k, int32_t gamma2);
unsigned int polyveck_make_hint(polyveck *h, const polyveck *v0,
                                const polyveck *v1, int k, int32_t gamma2);
void polyveck_use_hint(polyveck *w, const polyveck *u, const polyveck *h,
                       int k, int32_t gamma2);

/*************************************************
* Name:        polyvec_matrix_pointwise
*
* Description: Matrix-vector multiplication t = A * v using schoolbook
*              polynomial multiplication.
*
* Arguments:   - polyveck *t: pointer to output vector (length k)
*              - const polymat *A: pointer to k×l matrix
*              - const polyvecl *v: pointer to input vector (length l)
*              - int k: number of rows
*              - int l: number of columns
**************************************************/
void polyvec_matrix_pointwise(polyveck *t, const polymat *A,
                              const polyvecl *v, int k, int l);

#endif /* DILITHIUM_POLY_H */
