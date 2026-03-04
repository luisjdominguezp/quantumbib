/*
 * ML-DSA Key Generation  (ML-DSA.KeyGen_internal, NIST FIPS 204 §5.1)
 * Author : Jorge Ramón Figueroa Maya  –  PAP II, Week 6
 */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/evp.h>
#include "dilithium_keygen.h"

/* ── Internal: SHAKE-256 helper ────────────────────────────────────────── */

static void shake256_hash(uint8_t *out, size_t outlen,
                          const uint8_t *in, size_t inlen)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_shake256(), NULL);
    EVP_DigestUpdate(ctx, in, inlen);
    EVP_DigestFinalXOF(ctx, out, outlen);
    EVP_MD_CTX_free(ctx);
}

/* ── Core key generation ────────────────────────────────────────────────── */

int dilithium_keygen_from_seed(dilithium_pk *pk, dilithium_sk *sk,
                               const uint8_t seed[32])
{
    /* ── Step 1: derive (rho, rho_prime, K) from seed ──────────────────── *
     *   FIPS 204 §5.1 feeds  seed ‖ k ‖ l  into SHAKE-256 → 128 bytes.   *
     *   rho[0..31], rho_prime[32..95], K[96..127]                          */
    uint8_t h_input[34];
    uint8_t h_out[128];

    memcpy(h_input, seed, 32);
    h_input[32] = (uint8_t)DLT_K;
    h_input[33] = (uint8_t)DLT_L;
    shake256_hash(h_out, sizeof(h_out), h_input, 34);

    memcpy(pk->rho,        h_out,      32);
    memcpy(sk->rho_prime,  h_out + 32, 64);
    memcpy(sk->K,          h_out + 96, 32);
    memcpy(sk->rho,        pk->rho,    32);

    /* ── Step 2: expand matrix A from rho ──────────────────────────────── */
    polymat A;
    expand_matrix(&A, pk->rho, DLT_K, DLT_L);

    /* ── Step 3: sample small secret vectors s1 (length l) and s2 (k) ─── */
    for (int i = 0; i < DLT_L; i++)
        poly_uniform_eta(&sk->s1.vec[i], sk->rho_prime,
                         (uint16_t)i, DLT_ETA);

    for (int i = 0; i < DLT_K; i++)
        poly_uniform_eta(&sk->s2.vec[i], sk->rho_prime,
                         (uint16_t)(DLT_L + i), DLT_ETA);

    /* ── Step 4: compute t = A·s1 + s2 ─────────────────────────────────── */
    polyvecl_matrix_mul(&pk->t, &A, &sk->s1, DLT_K, DLT_L);

    for (int i = 0; i < DLT_K; i++) {
        poly_add(&pk->t.vec[i], &pk->t.vec[i], &sk->s2.vec[i]);
        poly_reduce(&pk->t.vec[i]);
        poly_caddq(&pk->t.vec[i]);   /* normalise to [0, q) for decompose */
    }

    /* ── Step 5: decompose t into (t1, t0) ─────────────────────────────── */
    for (int i = 0; i < DLT_K; i++) {
        poly_power2round(&pk->t1.vec[i], &sk->t0.vec[i], &pk->t.vec[i]);
        sk->t1.vec[i] = pk->t1.vec[i];
    }

    /* ── Step 6: compute tr = SHAKE-256(rho ‖ raw(t1), 64) ─────────────── *
     *   In the spec this is H(ByteEncode(pk)); we hash the raw coefficients *
     *   which suffices for correctness of our unpacked implementation.      */
    {
        size_t tr_in_len = 32 + (size_t)DLT_K * DILITHIUM_N * sizeof(int32_t);
        uint8_t *tr_in = (uint8_t *)malloc(tr_in_len);
        memcpy(tr_in, pk->rho, 32);
        for (int i = 0; i < DLT_K; i++)
            memcpy(tr_in + 32 + i * DILITHIUM_N * sizeof(int32_t),
                   pk->t1.vec[i].coeffs,
                   DILITHIUM_N * sizeof(int32_t));
        shake256_hash(sk->tr, 64, tr_in, tr_in_len);
        free(tr_in);
    }

    return 0;
}

int dilithium_keygen(dilithium_pk *pk, dilithium_sk *sk)
{
    uint8_t seed[32];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    ssize_t r = read(fd, seed, 32);
    close(fd);
    if (r != 32) return -1;

    return dilithium_keygen_from_seed(pk, sk, seed);
}
