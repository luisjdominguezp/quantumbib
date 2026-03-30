/*
 * Dilithium key generation (FIPS 204, ML-DSA.KeyGen_internal).
 * Based on ref/sign.c from https://github.com/pq-crystals/dilithium
 */

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/evp.h>
#include "dilithium_keygen.h"

/* SHAKE-256 helper */

static void shake256_hash(uint8_t *out, size_t outlen,
                          const uint8_t *in,  size_t inlen)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_shake256(), NULL);
    EVP_DigestUpdate(ctx, in, inlen);
    EVP_DigestFinalXOF(ctx, out, outlen);
    EVP_MD_CTX_free(ctx);
}

/*************************************************
* Name:        dilithium_keygen_from_seed
*
* Description: Deterministic key generation from a 32-byte seed.
*              Follows FIPS 204 Algorithm 1 (ML-DSA.KeyGen_internal):
*
*              1. (rho || rho' || K) = SHAKE-256(seed || k || l, 128)
*              2. A = ExpandA(rho)
*              3. (s1, s2) = ExpandS(rho')
*              4. t = A*s1 + s2
*              5. (t1, t0) = Power2Round(t)
*              6. tr = SHAKE-256(rho || raw(t1), 64)
*
* Arguments:   - dilithium_pk *pk: pointer to output public key
*              - dilithium_sk *sk: pointer to output private key
*              - const uint8_t seed[32]: 32-byte random seed
*
* Returns 0 (always succeeds).
**************************************************/
int dilithium_keygen_from_seed(dilithium_pk *pk, dilithium_sk *sk,
                               const uint8_t seed[DILITHIUM_SEEDBYTES])
{
    /* Derive (rho, rho_prime, K) from seed || k || l */
    uint8_t h_input[DILITHIUM_SEEDBYTES + 2];
    uint8_t h_out[2*DILITHIUM_SEEDBYTES + DILITHIUM_CRHBYTES]; /* 128 */

    memcpy(h_input, seed, DILITHIUM_SEEDBYTES);
    h_input[DILITHIUM_SEEDBYTES]     = (uint8_t)DLT_K;
    h_input[DILITHIUM_SEEDBYTES + 1] = (uint8_t)DLT_L;
    shake256_hash(h_out, sizeof(h_out), h_input, sizeof(h_input));

    memcpy(pk->rho,       h_out,                          DILITHIUM_SEEDBYTES);
    memcpy(sk->rho_prime, h_out + DILITHIUM_SEEDBYTES,    DILITHIUM_CRHBYTES);
    memcpy(sk->K,         h_out + DILITHIUM_SEEDBYTES + DILITHIUM_CRHBYTES,
           DILITHIUM_SEEDBYTES);
    memcpy(sk->rho,       pk->rho, DILITHIUM_SEEDBYTES);

    /* Expand matrix A from rho */
    polymat A;
    expand_matrix(&A, pk->rho, DLT_K, DLT_L);

    /* Sample short vectors s1 (l polys) and s2 (k polys) */
    polyvecl_uniform_eta(&sk->s1, sk->rho_prime, 0, DLT_L, DLT_ETA);
    polyveck_uniform_eta(&sk->s2, sk->rho_prime, (uint16_t)DLT_L,
                         DLT_K, DLT_ETA);

    /* t = A*s1 + s2 */
    polyvec_matrix_pointwise(&pk->t, &A, &sk->s1, DLT_K, DLT_L);
    polyveck_add(&pk->t, &pk->t, &sk->s2, DLT_K);
    polyveck_reduce(&pk->t, DLT_K);
    polyveck_caddq(&pk->t, DLT_K);

    /* (t1, t0) = Power2Round(t) */
    polyveck_power2round(&pk->t1, &sk->t0, &pk->t, DLT_K);
    for (int i = 0; i < DLT_K; i++)
        sk->t1.vec[i] = pk->t1.vec[i];

    /* tr = SHAKE-256(rho || raw(t1), 64)
     * We hash raw coefficients instead of pkEncode since we're unpacked. */
    {
        size_t tr_in_len = DILITHIUM_SEEDBYTES
                         + (size_t)DLT_K * DILITHIUM_N * sizeof(int32_t);
        uint8_t *tr_in = (uint8_t *)malloc(tr_in_len);
        memcpy(tr_in, pk->rho, DILITHIUM_SEEDBYTES);
        for (int i = 0; i < DLT_K; i++)
            memcpy(tr_in + DILITHIUM_SEEDBYTES
                         + (size_t)i * DILITHIUM_N * sizeof(int32_t),
                   pk->t1.vec[i].coeffs,
                   DILITHIUM_N * sizeof(int32_t));
        shake256_hash(sk->tr, DILITHIUM_TRBYTES, tr_in, tr_in_len);
        free(tr_in);
    }

    return 0;
}

/*************************************************
* Name:        dilithium_keygen
*
* Description: Generate a fresh ML-DSA key pair.
*              Reads DILITHIUM_SEEDBYTES random bytes from /dev/urandom.
*
* Arguments:   - dilithium_pk *pk: pointer to output public key
*              - dilithium_sk *sk: pointer to output private key
*
* Returns 0 on success, -1 if /dev/urandom is unavailable.
**************************************************/
/* Versión schoolbook de keygen — misma lógica pero usa polyvec_matrix_sb */
int dilithium_keygen_sb(dilithium_pk *pk, dilithium_sk *sk,
                        const uint8_t seed[DILITHIUM_SEEDBYTES])
{
    uint8_t h_input[DILITHIUM_SEEDBYTES + 2];
    uint8_t h_out[2*DILITHIUM_SEEDBYTES + DILITHIUM_CRHBYTES];

    memcpy(h_input, seed, DILITHIUM_SEEDBYTES);
    h_input[DILITHIUM_SEEDBYTES]     = (uint8_t)DLT_K;
    h_input[DILITHIUM_SEEDBYTES + 1] = (uint8_t)DLT_L;
    shake256_hash(h_out, sizeof(h_out), h_input, sizeof(h_input));

    memcpy(pk->rho,       h_out,                          DILITHIUM_SEEDBYTES);
    memcpy(sk->rho_prime, h_out + DILITHIUM_SEEDBYTES,    DILITHIUM_CRHBYTES);
    memcpy(sk->K,         h_out + DILITHIUM_SEEDBYTES + DILITHIUM_CRHBYTES,
           DILITHIUM_SEEDBYTES);
    memcpy(sk->rho, pk->rho, DILITHIUM_SEEDBYTES);

    polymat A;
    expand_matrix(&A, pk->rho, DLT_K, DLT_L);

    polyvecl_uniform_eta(&sk->s1, sk->rho_prime, 0, DLT_L, DLT_ETA);
    polyveck_uniform_eta(&sk->s2, sk->rho_prime, (uint16_t)DLT_L,
                         DLT_K, DLT_ETA);

    /* t = A*s1 + s2  — con schoolbook */
    polyvec_matrix_sb(&pk->t, &A, &sk->s1, DLT_K, DLT_L);
    polyveck_add(&pk->t, &pk->t, &sk->s2, DLT_K);
    polyveck_reduce(&pk->t, DLT_K);
    polyveck_caddq(&pk->t, DLT_K);

    polyveck_power2round(&pk->t1, &sk->t0, &pk->t, DLT_K);
    for (int i = 0; i < DLT_K; i++)
        sk->t1.vec[i] = pk->t1.vec[i];

    {
        size_t tr_in_len = DILITHIUM_SEEDBYTES
                         + (size_t)DLT_K * DILITHIUM_N * sizeof(int32_t);
        uint8_t *tr_in = (uint8_t *)malloc(tr_in_len);
        memcpy(tr_in, pk->rho, DILITHIUM_SEEDBYTES);
        for (int i = 0; i < DLT_K; i++)
            memcpy(tr_in + DILITHIUM_SEEDBYTES
                         + (size_t)i * DILITHIUM_N * sizeof(int32_t),
                   pk->t1.vec[i].coeffs,
                   DILITHIUM_N * sizeof(int32_t));
        shake256_hash(sk->tr, DILITHIUM_TRBYTES, tr_in, tr_in_len);
        free(tr_in);
    }

    return 0;
}

int dilithium_keygen(dilithium_pk *pk, dilithium_sk *sk)
{
    uint8_t seed[DILITHIUM_SEEDBYTES];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    ssize_t r = read(fd, seed, DILITHIUM_SEEDBYTES);
    close(fd);
    if (r != DILITHIUM_SEEDBYTES) return -1;

    return dilithium_keygen_from_seed(pk, sk, seed);
}
