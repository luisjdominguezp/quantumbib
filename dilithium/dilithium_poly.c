/*
 * Polynomial arithmetic and sampling for ML-DSA (CRYSTALS-Dilithium)
 * Standard : NIST FIPS 204
 * Author   : Jorge Ramón Figueroa Maya  –  PAP II, Week 6
 *
 * SHAKE-128 and SHAKE-256 are provided by OpenSSL (already linked as
 * -lssl -lcrypto) via the EVP extensible output function (XOF) interface.
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <openssl/evp.h>
#include "dilithium_poly.h"

/* ── Internal SHAKE helpers ────────────────────────────────────────────── */

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

/* ── Basic polynomial arithmetic ───────────────────────────────────────── */

void poly_add(poly *c, const poly *a, const poly *b)
{
    for (int i = 0; i < DILITHIUM_N; i++)
        c->coeffs[i] = a->coeffs[i] + b->coeffs[i];
}

void poly_sub(poly *c, const poly *a, const poly *b)
{
    for (int i = 0; i < DILITHIUM_N; i++)
        c->coeffs[i] = a->coeffs[i] - b->coeffs[i];
}

void poly_reduce(poly *a)
{
    for (int i = 0; i < DILITHIUM_N; i++)
        a->coeffs[i] = dilithium_reduce32(a->coeffs[i]);
}

void poly_caddq(poly *a)
{
    for (int i = 0; i < DILITHIUM_N; i++)
        a->coeffs[i] = dilithium_caddq(a->coeffs[i]);
}

/*
 * Schoolbook convolution in R_q = Z_q[X] / (X^N + 1).
 *
 * Exploits X^N ≡ -1:  coefficient k of the product is
 *   c[k] = Σ_{j≤k} a[j]·b[k-j]  −  Σ_{j>k} a[j]·b[N+k-j]
 *
 * Each term |a[i]·b[j]| < q² < 2^46, and we sum N=256 terms, so the
 * accumulator reaches at most 256·q² ≈ 2^54, well within int64_t.
 */
void poly_schoolbook_mul(poly *c, const poly *a, const poly *b)
{
    int64_t tmp[DILITHIUM_N];
    memset(tmp, 0, sizeof(tmp));

    for (int i = 0; i < DILITHIUM_N; i++) {
        for (int j = 0; j < DILITHIUM_N; j++) {
            int64_t prod = (int64_t)a->coeffs[i] * b->coeffs[j];
            int k = i + j;
            if (k < DILITHIUM_N)
                tmp[k] += prod;
            else
                tmp[k - DILITHIUM_N] -= prod;
        }
    }
    for (int i = 0; i < DILITHIUM_N; i++)
        c->coeffs[i] = dilithium_reduce32((int32_t)(tmp[i] % (int64_t)DILITHIUM_Q));
}

int32_t poly_inf_norm(const poly *a)
{
    int32_t norm = 0;
    for (int i = 0; i < DILITHIUM_N; i++) {
        int32_t v = a->coeffs[i];
        if (v < 0) v = -v;
        if (v > norm) norm = v;
    }
    return norm;
}

/* ── Decomposition helpers ─────────────────────────────────────────────── */

/*
 * Single-coefficient decompose:  r = r1·alpha + r0  mod q
 *   alpha = 2·gamma2
 *   r0 centred in (-gamma2, gamma2]
 *   Edge case: r1 = (q-1)/alpha → r1 = 0, r0 -= 1
 * Returns r1.
 */
static int32_t coeff_decompose(int32_t *r0, int32_t r, int32_t gamma2)
{
    int32_t alpha = 2 * gamma2;
    int32_t r1;

    /* Ensure r ∈ [0, q) */
    r = dilithium_caddq(dilithium_reduce32(r));

    *r0 = r % alpha;
    if (*r0 > gamma2) *r0 -= alpha;    /* centre into (-gamma2, gamma2] */

    r1 = (r - *r0) / alpha;

    /* Edge case per FIPS 204 Algorithm 36 */
    if (r1 == (int32_t)((DILITHIUM_Q - 1) / alpha)) {
        r1 = 0;
        *r0 -= 1;
    }
    return r1;
}

void poly_power2round(poly *t1, poly *t0, const poly *t)
{
    const int32_t half = 1 << (DILITHIUM_D - 1);   /* 2^12 = 4096 */

    for (int i = 0; i < DILITHIUM_N; i++) {
        int32_t r = dilithium_caddq(dilithium_reduce32(t->coeffs[i]));
        int32_t r0 = r & ((1 << DILITHIUM_D) - 1);
        if (r0 > half) r0 -= (1 << DILITHIUM_D);
        t0->coeffs[i] = r0;
        t1->coeffs[i] = (r - r0) >> DILITHIUM_D;
    }
}

void poly_highbits(poly *r1, const poly *r, int32_t gamma2)
{
    for (int i = 0; i < DILITHIUM_N; i++) {
        int32_t r0;
        r1->coeffs[i] = coeff_decompose(&r0, r->coeffs[i], gamma2);
    }
}

void poly_lowbits(poly *r0, const poly *r, int32_t gamma2)
{
    for (int i = 0; i < DILITHIUM_N; i++) {
        coeff_decompose(&r0->coeffs[i], r->coeffs[i], gamma2);
    }
}

int poly_makehint(poly *h, const poly *z, const poly *r, int32_t gamma2)
{
    int32_t alpha = 2 * gamma2;
    int32_t m = (DILITHIUM_Q - 1) / alpha;
    int cnt = 0;

    for (int i = 0; i < DILITHIUM_N; i++) {
        int32_t r0_unused;
        int32_t rz = dilithium_caddq(dilithium_reduce32(
                                        r->coeffs[i] + z->coeffs[i]));
        int32_t r1  = coeff_decompose(&r0_unused, r->coeffs[i], gamma2);
        int32_t rz1 = coeff_decompose(&r0_unused, rz,           gamma2);
        /* Wrap r1 into [0, m) */
        r1  = ((r1  % m) + m) % m;
        rz1 = ((rz1 % m) + m) % m;
        h->coeffs[i] = (r1 != rz1) ? 1 : 0;
        cnt += h->coeffs[i];
    }
    return cnt;   /* total number of hint bits set */
}

void poly_usehint(poly *w1, const poly *h, const poly *r, int32_t gamma2)
{
    int32_t alpha = 2 * gamma2;
    int32_t m = (DILITHIUM_Q - 1) / alpha;

    for (int i = 0; i < DILITHIUM_N; i++) {
        int32_t r0, r1;
        r1 = coeff_decompose(&r0, r->coeffs[i], gamma2);
        r1 = ((r1 % m) + m) % m;

        if (h->coeffs[i] == 0) {
            w1->coeffs[i] = r1;
        } else if (r0 > 0) {
            w1->coeffs[i] = (r1 + 1) % m;
        } else {
            w1->coeffs[i] = (r1 - 1 + m) % m;
        }
    }
}

/* ── Polynomial sampling ───────────────────────────────────────────────── */

/*
 * poly_uniform  –  RejNTTPoly (FIPS 204 Algorithm 30)
 *
 * Feeds (seed ‖ nonce_lo ‖ nonce_hi) into SHAKE-128 and extracts
 * coefficients via rejection sampling on 23-bit values.
 * Expected buffer consumption: ~768 bytes for N=256, q ≈ 2^23.
 */
void poly_uniform(poly *a, const uint8_t seed[32], uint16_t nonce)
{
    /* 864 bytes ≈ 5 × SHAKE-128 rate (168); covers >99.9% of cases */
    uint8_t buf[864];
    uint8_t input[34];

    memcpy(input, seed, 32);
    input[32] = (uint8_t)(nonce & 0xFF);
    input[33] = (uint8_t)(nonce >> 8);
    shake128(buf, sizeof(buf), input, 34);

    int ctr = 0;
    size_t pos = 0;
    while (ctr < DILITHIUM_N && pos + 3 <= sizeof(buf)) {
        uint32_t t  = (uint32_t)buf[pos]
                    | ((uint32_t)buf[pos + 1] << 8)
                    | ((uint32_t)buf[pos + 2] << 16);
        t &= 0x7FFFFF;   /* keep 23 bits */
        pos += 3;
        if (t < (uint32_t)DILITHIUM_Q)
            a->coeffs[ctr++] = (int32_t)t;
    }
    /* If the buffer was exhausted (astronomically unlikely), zero remaining */
    while (ctr < DILITHIUM_N)
        a->coeffs[ctr++] = 0;
}

/*
 * poly_uniform_eta  –  RejBoundedPoly (FIPS 204 Algorithm 31)
 *
 * Nibble-based rejection sampling.
 *   eta = 2: accept nibble t if t < 15; coeff = 2 − (t mod 5)
 *   eta = 4: accept nibble t if t < 9;  coeff = 4 − t
 */
void poly_uniform_eta(poly *a, const uint8_t seed[64],
                      uint16_t nonce, int eta)
{
    /* 136 bytes is more than enough for both eta values */
    uint8_t buf[272];
    uint8_t input[66];

    memcpy(input, seed, 64);
    input[64] = (uint8_t)(nonce & 0xFF);
    input[65] = (uint8_t)(nonce >> 8);
    shake256(buf, sizeof(buf), input, 66);

    int ctr = 0;
    size_t pos = 0;
    while (ctr < DILITHIUM_N && pos < sizeof(buf)) {
        uint8_t byte = buf[pos++];
        uint8_t lo   = byte & 0x0F;
        uint8_t hi   = byte >> 4;

        if (eta == 2) {
            if (lo < 15) a->coeffs[ctr++] = 2 - (lo % 5);
            if (ctr < DILITHIUM_N && hi < 15)
                a->coeffs[ctr++] = 2 - (hi % 5);
        } else {   /* eta == 4 */
            if (lo < 9) a->coeffs[ctr++] = 4 - (int32_t)lo;
            if (ctr < DILITHIUM_N && hi < 9)
                a->coeffs[ctr++] = 4 - (int32_t)hi;
        }
    }
    while (ctr < DILITHIUM_N)
        a->coeffs[ctr++] = 0;
}

/*
 * poly_uniform_gamma1  –  ExpandMask (FIPS 204 Algorithm 32)
 *
 * Extracts (bits) bits per coefficient using a sequential bit reader
 * on SHAKE-256 output, then maps to (-gamma1, gamma1] via
 *   coeff = gamma1 − unsigned_value.
 *
 *   gamma1 = 2^17 → bits = 18  (unsigned range [0, 2^18), no rejection)
 *   gamma1 = 2^19 → bits = 20  (unsigned range [0, 2^20), no rejection)
 */
void poly_uniform_gamma1(poly *a, const uint8_t seed[64],
                         uint16_t nonce, int32_t gamma1)
{
    int bits = (gamma1 == (1 << 17)) ? 18 : 20;
    size_t buflen = (size_t)((DILITHIUM_N * bits + 7) / 8) + 64;

    uint8_t *buf = (uint8_t *)malloc(buflen);
    uint8_t input[66];

    memcpy(input, seed, 64);
    input[64] = (uint8_t)(nonce & 0xFF);
    input[65] = (uint8_t)(nonce >> 8);
    shake256(buf, buflen, input, 66);

    uint32_t bit_buf = 0;
    int bit_buf_len  = 0;
    size_t byte_pos  = 0;
    uint32_t mask    = ((uint32_t)1 << bits) - 1;

    for (int i = 0; i < DILITHIUM_N; i++) {
        while (bit_buf_len < bits && byte_pos < buflen) {
            bit_buf    |= (uint32_t)buf[byte_pos++] << bit_buf_len;
            bit_buf_len += 8;
        }
        a->coeffs[i] = gamma1 - (int32_t)(bit_buf & mask);
        bit_buf    >>= bits;
        bit_buf_len -= bits;
    }
    free(buf);
}

/*
 * poly_challenge  –  SampleInBall (FIPS 204 Algorithm 33)
 *
 * Fisher-Yates shuffle: places tau ±1 entries into positions
 * [N-tau, N-1], signs determined by the leading 64 bits of SHAKE-256.
 */
void poly_challenge(poly *c, const uint8_t seed[32], int tau)
{
    uint8_t buf[512];
    shake256(buf, sizeof(buf), seed, 32);

    /* Extract sign bits from the first 8 bytes */
    uint64_t signs = 0;
    for (int i = 0; i < 8; i++)
        signs |= (uint64_t)buf[i] << (8 * i);

    memset(c->coeffs, 0, sizeof(c->coeffs));

    int pos = 8;
    for (int i = DILITHIUM_N - tau; i < DILITHIUM_N; i++) {
        int b;
        do {
            b = (int)buf[pos++];
            if (pos >= (int)sizeof(buf)) pos = 8; /* wrap (never in practice) */
        } while (b > i);

        c->coeffs[i]       = c->coeffs[b];
        c->coeffs[b]       = 1 - 2 * (int32_t)(signs & 1);
        signs            >>= 1;
    }
}

/* ── Matrix / vector operations ────────────────────────────────────────── */

void expand_matrix(polymat *A, const uint8_t rho[32], int k, int l)
{
    for (int i = 0; i < k; i++)
        for (int j = 0; j < l; j++)
            poly_uniform(&A->mat[i][j], rho, (uint16_t)((i << 8) | j));
}

void polyvecl_matrix_mul(polyveck *t, const polymat *A,
                         const polyvecl *v, int k, int l)
{
    poly tmp;
    for (int i = 0; i < k; i++) {
        memset(t->vec[i].coeffs, 0, sizeof(t->vec[i].coeffs));
        for (int j = 0; j < l; j++) {
            poly_schoolbook_mul(&tmp, &A->mat[i][j], &v->vec[j]);
            poly_add(&t->vec[i], &t->vec[i], &tmp);
        }
        poly_reduce(&t->vec[i]);
    }
}

void poly_vec_add(poly *c, const poly *a, const poly *b, int n)
{
    for (int i = 0; i < n; i++)
        poly_add(&c[i], &a[i], &b[i]);
}

void poly_vec_sub(poly *c, const poly *a, const poly *b, int n)
{
    for (int i = 0; i < n; i++)
        poly_sub(&c[i], &a[i], &b[i]);
}

void poly_vec_mul_scalar(poly *c, const poly *scalar, const poly *v, int n)
{
    for (int i = 0; i < n; i++)
        poly_schoolbook_mul(&c[i], scalar, &v[i]);
}

void poly_vec_reduce(poly *v, int n)
{
    for (int i = 0; i < n; i++)
        poly_reduce(&v[i]);
}

void poly_vec_caddq(poly *v, int n)
{
    for (int i = 0; i < n; i++)
        poly_caddq(&v[i]);
}

int32_t poly_vec_inf_norm(const poly *v, int n)
{
    int32_t norm = 0;
    for (int i = 0; i < n; i++) {
        int32_t ni = poly_inf_norm(&v[i]);
        if (ni > norm) norm = ni;
    }
    return norm;
}
