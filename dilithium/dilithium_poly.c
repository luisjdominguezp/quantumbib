/*
 * Polynomial arithmetic, rounding and sampling for Dilithium.
 * Merges ref/poly.c, ref/polyvec.c and ref/rounding.c from
 * https://github.com/pq-crystals/dilithium
 *
 * Uses schoolbook multiplication instead of NTT, and OpenSSL
 * for SHAKE-128/256 instead of the bundled fips202.c.
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <openssl/evp.h>
#include "dilithium_poly.h"
#include "dilithium_ntt.h"

/* Internal SHAKE helpers (using OpenSSL instead of fips202.c) */

static void shake128(uint8_t *out, size_t outlen,
                     const uint8_t *in,  size_t inlen)
{
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, EVP_shake128(), NULL);
  EVP_DigestUpdate(ctx, in, inlen);
  EVP_DigestFinalXOF(ctx, out, outlen);
  EVP_MD_CTX_free(ctx);
}

static void shake256(uint8_t *out, size_t outlen,
                     const uint8_t *in,  size_t inlen)
{
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, EVP_shake256(), NULL);
  EVP_DigestUpdate(ctx, in, inlen);
  EVP_DigestFinalXOF(ctx, out, outlen);
  EVP_MD_CTX_free(ctx);
}

/* Polynomial arithmetic */

/*************************************************
* Name:        poly_reduce
*
* Description: Inplace reduction of all coefficients of polynomial to
*              representative in [-6283008,6283008].
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
void poly_reduce(poly *a) {
  unsigned int i;
  for(i = 0; i < DILITHIUM_N; ++i)
    a->coeffs[i] = dilithium_reduce32(a->coeffs[i]);
}

/*************************************************
* Name:        poly_caddq
*
* Description: For all coefficients of in/out polynomial add Q if
*              coefficient is negative.
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
void poly_caddq(poly *a) {
  unsigned int i;
  for(i = 0; i < DILITHIUM_N; ++i)
    a->coeffs[i] = dilithium_caddq(a->coeffs[i]);
}

/*************************************************
* Name:        poly_add
*
* Description: Add polynomials. No modular reduction is performed.
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const poly *a: pointer to first summand
*              - const poly *b: pointer to second summand
**************************************************/
void poly_add(poly *c, const poly *a, const poly *b) {
  unsigned int i;
  for(i = 0; i < DILITHIUM_N; ++i)
    c->coeffs[i] = a->coeffs[i] + b->coeffs[i];
}

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
void poly_sub(poly *c, const poly *a, const poly *b) {
  unsigned int i;
  for(i = 0; i < DILITHIUM_N; ++i)
    c->coeffs[i] = a->coeffs[i] - b->coeffs[i];
}

/*************************************************
* Name:        poly_shiftl
*
* Description: Multiply polynomial by 2^D without modular reduction.
*              Assumes input coefficients to be less than 2^{31-D} in
*              absolute value.
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
void poly_shiftl(poly *a) {
  unsigned int i;
  for(i = 0; i < DILITHIUM_N; ++i)
    a->coeffs[i] <<= DILITHIUM_D;
}

/*************************************************
* Name:        poly_schoolbook_mul
*
* Description: Schoolbook O(N^2) polynomial multiplication in
*              R_q = Z_q[X]/(X^N+1).  Uses X^N = -1 to fold the
*              upper half of the convolution.  Each product
*              |a[i]*b[j]| < q^2 < 2^46, and we sum N=256 terms,
*              so the accumulator reaches at most 256*q^2 ~ 2^54,
*              well within int64_t range.
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const poly *a: pointer to first input polynomial
*              - const poly *b: pointer to second input polynomial
**************************************************/
void poly_schoolbook_mul(poly *c, const poly *a, const poly *b) {
  unsigned int i, j;
  int64_t tmp[DILITHIUM_N];
  memset(tmp, 0, sizeof(tmp));

  for(i = 0; i < DILITHIUM_N; ++i) {
    for(j = 0; j < DILITHIUM_N; ++j) {
      int64_t prod = (int64_t)a->coeffs[i] * b->coeffs[j];
      unsigned int k = i + j;
      if(k < DILITHIUM_N)
        tmp[k] += prod;
      else
        tmp[k - DILITHIUM_N] -= prod;
    }
  }
  for(i = 0; i < DILITHIUM_N; ++i)
    c->coeffs[i] = dilithium_reduce32((int32_t)(tmp[i] % (int64_t)DILITHIUM_Q));
}

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
int poly_chknorm(const poly *a, int32_t B) {
  unsigned int i;
  int32_t t;

  if(B > (DILITHIUM_Q - 1) / 8)
    return 1;

  for(i = 0; i < DILITHIUM_N; ++i) {
    /* Absolute value */
    t = a->coeffs[i] >> 31;
    t = a->coeffs[i] - (t & 2*a->coeffs[i]);

    if(t >= B)
      return 1;
  }

  return 0;
}

/* Rounding */

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
int32_t power2round(int32_t *a0, int32_t a) {
  int32_t a1;

  a1 = (a + (1 << (DILITHIUM_D - 1)) - 1) >> DILITHIUM_D;
  *a0 = a - (a1 << DILITHIUM_D);
  return a1;
}

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
int32_t decompose(int32_t *a0, int32_t a, int32_t gamma2) {
  int32_t a1;

  a1  = (a + 127) >> 7;
  if(gamma2 == (DILITHIUM_Q - 1) / 32) {
    a1  = (a1*1025 + (1 << 21)) >> 22;
    a1 &= 15;
  }
  else if(gamma2 == (DILITHIUM_Q - 1) / 88) {
    a1  = (a1*11275 + (1 << 23)) >> 24;
    a1 ^= ((43 - a1) >> 31) & a1;
  }

  *a0  = a - a1*2*gamma2;
  *a0 -= (((DILITHIUM_Q - 1) / 2 - *a0) >> 31) & DILITHIUM_Q;
  return a1;
}

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
unsigned int make_hint(int32_t a0, int32_t a1, int32_t gamma2) {
  if(a0 > gamma2 || a0 < -gamma2 || (a0 == -gamma2 && a1 != 0))
    return 1;

  return 0;
}

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
int32_t use_hint(int32_t a, unsigned int hint, int32_t gamma2) {
  int32_t a0, a1;

  a1 = decompose(&a0, a, gamma2);
  if(hint == 0)
    return a1;

  if(gamma2 == (DILITHIUM_Q - 1) / 32) {
    if(a0 > 0)
      return (a1 + 1) & 15;
    else
      return (a1 - 1) & 15;
  }
  else {  /* gamma2 == (Q-1)/88 */
    if(a0 > 0)
      return (a1 == 43) ?  0 : a1 + 1;
    else
      return (a1 ==  0) ? 43 : a1 - 1;
  }
}

/* Polynomial-level rounding wrappers */

void poly_power2round(poly *a1, poly *a0, const poly *a) {
  unsigned int i;
  for(i = 0; i < DILITHIUM_N; ++i)
    a1->coeffs[i] = power2round(&a0->coeffs[i], a->coeffs[i]);
}

void poly_decompose(poly *a1, poly *a0, const poly *a, int32_t gamma2) {
  unsigned int i;
  for(i = 0; i < DILITHIUM_N; ++i)
    a1->coeffs[i] = decompose(&a0->coeffs[i], a->coeffs[i], gamma2);
}

unsigned int poly_make_hint(poly *h, const poly *a0, const poly *a1,
                            int32_t gamma2) {
  unsigned int i, s = 0;
  for(i = 0; i < DILITHIUM_N; ++i) {
    h->coeffs[i] = make_hint(a0->coeffs[i], a1->coeffs[i], gamma2);
    s += h->coeffs[i];
  }
  return s;
}

void poly_use_hint(poly *b, const poly *a, const poly *h, int32_t gamma2) {
  unsigned int i;
  for(i = 0; i < DILITHIUM_N; ++i)
    b->coeffs[i] = use_hint(a->coeffs[i], h->coeffs[i], gamma2);
}

/* Sampling */

/*************************************************
* Name:        rej_uniform
*
* Description: Sample uniformly random coefficients in [0, Q-1] by
*              performing rejection sampling on array of random bytes.
*
* Arguments:   - int32_t *a: pointer to output array (allocated)
*              - unsigned int len: number of coefficients to be sampled
*              - const uint8_t *buf: array of random bytes
*              - unsigned int buflen: length of array of random bytes
*
* Returns number of sampled coefficients. Can be smaller than len
* if not enough random bytes were given.
**************************************************/
static unsigned int rej_uniform(int32_t *a,
                                unsigned int len,
                                const uint8_t *buf,
                                unsigned int buflen)
{
  unsigned int ctr, pos;
  uint32_t t;

  ctr = pos = 0;
  while(ctr < len && pos + 3 <= buflen) {
    t  = buf[pos++];
    t |= (uint32_t)buf[pos++] << 8;
    t |= (uint32_t)buf[pos++] << 16;
    t &= 0x7FFFFF;

    if(t < (uint32_t)DILITHIUM_Q)
      a[ctr++] = t;
  }

  return ctr;
}

/*************************************************
* Name:        poly_uniform
*
* Description: Sample polynomial with uniformly random coefficients
*              in [0,Q-1] by performing rejection sampling on the
*              output stream of SHAKE128(seed|nonce).
*
* Arguments:   - poly *a: pointer to output polynomial
*              - const uint8_t seed[]: byte array with seed
*              - uint16_t nonce: 2-byte nonce
**************************************************/
#define POLY_UNIFORM_BUFLEN  (5*168)  /* 840 bytes: 5 SHAKE-128 blocks */
void poly_uniform(poly *a,
                  const uint8_t seed[DILITHIUM_SEEDBYTES],
                  uint16_t nonce)
{
  unsigned int ctr;
  uint8_t buf[POLY_UNIFORM_BUFLEN + 2];
  uint8_t input[DILITHIUM_SEEDBYTES + 2];

  memcpy(input, seed, DILITHIUM_SEEDBYTES);
  input[DILITHIUM_SEEDBYTES + 0] = (uint8_t)(nonce & 0xFF);
  input[DILITHIUM_SEEDBYTES + 1] = (uint8_t)(nonce >> 8);
  shake128(buf, POLY_UNIFORM_BUFLEN, input, DILITHIUM_SEEDBYTES + 2);

  ctr = rej_uniform(a->coeffs, DILITHIUM_N, buf, POLY_UNIFORM_BUFLEN);

  /* Extremely unlikely: buffer exhausted before N coefficients */
  while(ctr < DILITHIUM_N)
    a->coeffs[ctr++] = 0;
}

/*************************************************
* Name:        rej_eta
*
* Description: Sample uniformly random coefficients in [-ETA, ETA] by
*              performing rejection sampling on array of random bytes.
*
* Arguments:   - int32_t *a: pointer to output array
*              - unsigned int len: number of coefficients to be sampled
*              - const uint8_t *buf: array of random bytes
*              - unsigned int buflen: length of array
*              - int eta: bound on coefficients (2 or 4)
*
* Returns number of sampled coefficients.
**************************************************/
static unsigned int rej_eta(int32_t *a,
                            unsigned int len,
                            const uint8_t *buf,
                            unsigned int buflen,
                            int eta)
{
  unsigned int ctr, pos;
  uint32_t t0, t1;

  ctr = pos = 0;
  while(ctr < len && pos < buflen) {
    t0 = buf[pos] & 0x0F;
    t1 = buf[pos++] >> 4;

    if(eta == 2) {
      if(t0 < 15) {
        t0 = t0 - (205*t0 >> 10)*5;
        a[ctr++] = 2 - t0;
      }
      if(t1 < 15 && ctr < len) {
        t1 = t1 - (205*t1 >> 10)*5;
        a[ctr++] = 2 - t1;
      }
    } else {  /* eta == 4 */
      if(t0 < 9)
        a[ctr++] = 4 - t0;
      if(t1 < 9 && ctr < len)
        a[ctr++] = 4 - t1;
    }
  }

  return ctr;
}

/*************************************************
* Name:        poly_uniform_eta
*
* Description: Sample polynomial with uniformly random coefficients
*              in [-ETA,ETA] by performing rejection sampling on the
*              output stream from SHAKE256(seed|nonce).
*
* Arguments:   - poly *a: pointer to output polynomial
*              - const uint8_t seed[]: byte array with seed
*              - uint16_t nonce: 2-byte nonce
*              - int eta: bound on coefficients (2 or 4)
**************************************************/
void poly_uniform_eta(poly *a,
                      const uint8_t seed[DILITHIUM_CRHBYTES],
                      uint16_t nonce, int eta)
{
  uint8_t buf[272];
  uint8_t input[DILITHIUM_CRHBYTES + 2];

  memcpy(input, seed, DILITHIUM_CRHBYTES);
  input[DILITHIUM_CRHBYTES + 0] = (uint8_t)(nonce & 0xFF);
  input[DILITHIUM_CRHBYTES + 1] = (uint8_t)(nonce >> 8);
  shake256(buf, sizeof(buf), input, DILITHIUM_CRHBYTES + 2);

  rej_eta(a->coeffs, DILITHIUM_N, buf, sizeof(buf), eta);
}

/*************************************************
* Name:        poly_uniform_gamma1
*
* Description: Sample polynomial with uniformly random coefficients
*              in [-(GAMMA1-1), GAMMA1] by unpacking output stream of
*              SHAKE256(seed|nonce).
*
* Arguments:   - poly *a: pointer to output polynomial
*              - const uint8_t seed[]: byte array with seed
*              - uint16_t nonce: 16-bit nonce
*              - int32_t gamma1: bound on coefficients
**************************************************/
void poly_uniform_gamma1(poly *a,
                         const uint8_t seed[DILITHIUM_CRHBYTES],
                         uint16_t nonce, int32_t gamma1)
{
  int bits = (gamma1 == (1 << 17)) ? 18 : 20;
  size_t buflen = (size_t)((DILITHIUM_N * bits + 7) / 8) + 64;

  uint8_t *buf = (uint8_t *)malloc(buflen);
  uint8_t input[DILITHIUM_CRHBYTES + 2];

  memcpy(input, seed, DILITHIUM_CRHBYTES);
  input[DILITHIUM_CRHBYTES + 0] = (uint8_t)(nonce & 0xFF);
  input[DILITHIUM_CRHBYTES + 1] = (uint8_t)(nonce >> 8);
  shake256(buf, buflen, input, DILITHIUM_CRHBYTES + 2);

  uint32_t bit_buf = 0;
  int bit_buf_len  = 0;
  size_t byte_pos  = 0;
  uint32_t mask    = ((uint32_t)1 << bits) - 1;

  for(unsigned int i = 0; i < DILITHIUM_N; ++i) {
    while(bit_buf_len < bits && byte_pos < buflen) {
      bit_buf    |= (uint32_t)buf[byte_pos++] << bit_buf_len;
      bit_buf_len += 8;
    }
    a->coeffs[i] = gamma1 - (int32_t)(bit_buf & mask);
    bit_buf    >>= bits;
    bit_buf_len -= bits;
  }
  free(buf);
}

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
void poly_challenge(poly *c, const uint8_t *seed, int tau) {
  unsigned int i, b, pos;
  uint64_t signs;
  uint8_t buf[512];

  shake256(buf, sizeof(buf), seed, 32);

  signs = 0;
  for(i = 0; i < 8; ++i)
    signs |= (uint64_t)buf[i] << 8*i;
  pos = 8;

  for(i = 0; i < DILITHIUM_N; ++i)
    c->coeffs[i] = 0;
  for(i = DILITHIUM_N - tau; i < DILITHIUM_N; ++i) {
    do {
      if(pos >= sizeof(buf)) pos = 8; /* wrap (never in practice) */
      b = buf[pos++];
    } while(b > i);

    c->coeffs[i] = c->coeffs[b];
    c->coeffs[b] = 1 - 2*(signs & 1);
    signs >>= 1;
  }
}

/* Schoolbook vector/matrix ops — usan poly_schoolbook_mul en vez de NTT */

void polyvec_matrix_sb(polyveck *t, const polymat *A,
                       const polyvecl *v, int k, int l) {
  unsigned int i, j;
  poly tmp;
  for(i = 0; i < (unsigned)k; ++i) {
    memset(t->vec[i].coeffs, 0, sizeof(t->vec[i].coeffs));
    for(j = 0; j < (unsigned)l; ++j) {
      poly_schoolbook_mul(&tmp, &A->mat[i][j], &v->vec[j]);
      poly_add(&t->vec[i], &t->vec[i], &tmp);
    }
    poly_reduce(&t->vec[i]);
  }
}

void polyvecl_pointwise_poly_sb(polyvecl *r, const poly *a,
                                const polyvecl *v, int l) {
  unsigned int i;
  for(i = 0; i < (unsigned)l; ++i)
    poly_schoolbook_mul(&r->vec[i], a, &v->vec[i]);
}

void polyveck_pointwise_poly_sb(polyveck *r, const poly *a,
                                const polyveck *v, int k) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_schoolbook_mul(&r->vec[i], a, &v->vec[i]);
}

/* Vector / matrix operations */

/*************************************************
* Name:        expand_matrix
*
* Description: Implementation of ExpandA. Generates matrix A with uniformly
*              random coefficients a_{i,j} by performing rejection
*              sampling on the output stream of SHAKE128(rho|j|i).
*
* Arguments:   - polymat *A: output matrix
*              - const uint8_t rho[]: byte array containing seed rho
*              - int k: number of rows
*              - int l: number of columns
**************************************************/
void expand_matrix(polymat *A, const uint8_t rho[DILITHIUM_SEEDBYTES],
                   int k, int l) {
  unsigned int i, j;
  for(i = 0; i < (unsigned)k; ++i)
    for(j = 0; j < (unsigned)l; ++j)
      poly_uniform(&A->mat[i][j], rho, (uint16_t)((i << 8) + j));
}

/*************************************************
* Name:        polyvec_matrix_pointwise
*
* Description: Matrix-vector multiplication t = A * v using schoolbook
*              polynomial multiplication.
*
* Arguments:   - polyveck *t: output vector (length k)
*              - const polymat *A: k x l matrix
*              - const polyvecl *v: input vector (length l)
*              - int k: number of rows
*              - int l: number of columns
**************************************************/
void polyvec_matrix_pointwise(polyveck *t, const polymat *A,
                              const polyvecl *v, int k, int l) {
  unsigned int i, j;
  poly tmp;
  for(i = 0; i < (unsigned)k; ++i) {
    memset(t->vec[i].coeffs, 0, sizeof(t->vec[i].coeffs));
    for(j = 0; j < (unsigned)l; ++j) {
      poly_ntt_mul(&tmp, &A->mat[i][j], &v->vec[j]);
      poly_add(&t->vec[i], &t->vec[i], &tmp);
    }
    poly_reduce(&t->vec[i]);
  }
}

/************ Vectors of polynomials of length L **************/

void polyvecl_uniform_eta(polyvecl *v,
                          const uint8_t seed[DILITHIUM_CRHBYTES],
                          uint16_t nonce, int l, int eta) {
  unsigned int i;
  for(i = 0; i < (unsigned)l; ++i)
    poly_uniform_eta(&v->vec[i], seed, nonce++, eta);
}

void polyvecl_uniform_gamma1(polyvecl *v,
                             const uint8_t seed[DILITHIUM_CRHBYTES],
                             uint16_t nonce, int l, int32_t gamma1) {
  unsigned int i;
  for(i = 0; i < (unsigned)l; ++i)
    poly_uniform_gamma1(&v->vec[i], seed, (uint16_t)(l*nonce + i), gamma1);
}

void polyvecl_reduce(polyvecl *v, int l) {
  unsigned int i;
  for(i = 0; i < (unsigned)l; ++i)
    poly_reduce(&v->vec[i]);
}

void polyvecl_add(polyvecl *w, const polyvecl *u, const polyvecl *v,
                  int l) {
  unsigned int i;
  for(i = 0; i < (unsigned)l; ++i)
    poly_add(&w->vec[i], &u->vec[i], &v->vec[i]);
}

void polyvecl_sub(polyvecl *w, const polyvecl *u, const polyvecl *v,
                  int l) {
  unsigned int i;
  for(i = 0; i < (unsigned)l; ++i)
    poly_sub(&w->vec[i], &u->vec[i], &v->vec[i]);
}

void polyvecl_pointwise_poly(polyvecl *r, const poly *a,
                             const polyvecl *v, int l) {
  unsigned int i;
  for(i = 0; i < (unsigned)l; ++i)
    poly_ntt_mul(&r->vec[i], a, &v->vec[i]);
}

int polyvecl_chknorm(const polyvecl *v, int l, int32_t bound) {
  unsigned int i;
  for(i = 0; i < (unsigned)l; ++i)
    if(poly_chknorm(&v->vec[i], bound))
      return 1;
  return 0;
}

/************ Vectors of polynomials of length K **************/

void polyveck_reduce(polyveck *v, int k) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_reduce(&v->vec[i]);
}

void polyveck_caddq(polyveck *v, int k) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_caddq(&v->vec[i]);
}

void polyveck_add(polyveck *w, const polyveck *u, const polyveck *v,
                  int k) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_add(&w->vec[i], &u->vec[i], &v->vec[i]);
}

void polyveck_sub(polyveck *w, const polyveck *u, const polyveck *v,
                  int k) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_sub(&w->vec[i], &u->vec[i], &v->vec[i]);
}

void polyveck_uniform_eta(polyveck *v,
                          const uint8_t seed[DILITHIUM_CRHBYTES],
                          uint16_t nonce, int k, int eta) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_uniform_eta(&v->vec[i], seed, nonce++, eta);
}

void polyveck_shiftl(polyveck *v, int k) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_shiftl(&v->vec[i]);
}

void polyveck_pointwise_poly(polyveck *r, const poly *a,
                             const polyveck *v, int k) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_ntt_mul(&r->vec[i], a, &v->vec[i]);
}

int polyveck_chknorm(const polyveck *v, int k, int32_t bound) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    if(poly_chknorm(&v->vec[i], bound))
      return 1;
  return 0;
}

void polyveck_power2round(polyveck *v1, polyveck *v0, const polyveck *v,
                          int k) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_power2round(&v1->vec[i], &v0->vec[i], &v->vec[i]);
}

void polyveck_decompose(polyveck *v1, polyveck *v0, const polyveck *v,
                        int k, int32_t gamma2) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_decompose(&v1->vec[i], &v0->vec[i], &v->vec[i], gamma2);
}

unsigned int polyveck_make_hint(polyveck *h, const polyveck *v0,
                                const polyveck *v1, int k,
                                int32_t gamma2) {
  unsigned int i, s = 0;
  for(i = 0; i < (unsigned)k; ++i)
    s += poly_make_hint(&h->vec[i], &v0->vec[i], &v1->vec[i], gamma2);
  return s;
}

void polyveck_use_hint(polyveck *w, const polyveck *u, const polyveck *h,
                       int k, int32_t gamma2) {
  unsigned int i;
  for(i = 0; i < (unsigned)k; ++i)
    poly_use_hint(&w->vec[i], &u->vec[i], &h->vec[i], gamma2);
}

