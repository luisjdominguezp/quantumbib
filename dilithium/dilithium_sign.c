/*
 * Dilithium sign and verify (FIPS 204, Algorithms 2-3).
 * Based on ref/sign.c from https://github.com/pq-crystals/dilithium
 */

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/evp.h>
#include "dilithium_sign.h"

/* Internal helpers */

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
* Name:        recompute_tr
*
* Description: Recompute tr = SHAKE-256(rho || raw(t1), TRBYTES)
*              from the public key.
*
* Arguments:   - uint8_t *tr: output buffer (DILITHIUM_TRBYTES bytes)
*              - const dilithium_pk *pk: pointer to public key
**************************************************/
static void recompute_tr(uint8_t *tr, const dilithium_pk *pk)
{
    size_t len = DILITHIUM_SEEDBYTES
               + (size_t)DLT_K * DILITHIUM_N * sizeof(int32_t);
    uint8_t *buf = (uint8_t *)malloc(len);
    memcpy(buf, pk->rho, DILITHIUM_SEEDBYTES);
    for (int i = 0; i < DLT_K; i++)
        memcpy(buf + DILITHIUM_SEEDBYTES
                   + (size_t)i * DILITHIUM_N * sizeof(int32_t),
               pk->t1.vec[i].coeffs,
               DILITHIUM_N * sizeof(int32_t));
    shake256_hash(tr, DILITHIUM_TRBYTES, buf, len);
    free(buf);
}

/*************************************************
* Name:        hash_w1
*
* Description: Compute challenge hash c_tilde = SHAKE-256(mu || raw(w1)).
*              We hash raw coefficients since our representation is unpacked.
*
* Arguments:   - uint8_t *c_tilde: output (DLT_CTILDEBYTES bytes)
*              - const uint8_t mu[]: mu buffer (DILITHIUM_TRBYTES bytes)
*              - const polyveck *w1: pointer to high-bits vector
**************************************************/
static void hash_w1(uint8_t *c_tilde,
                    const uint8_t mu[DILITHIUM_TRBYTES],
                    const polyveck *w1)
{
    size_t w1_bytes = (size_t)DLT_K * DILITHIUM_N * sizeof(int32_t);
    size_t len = DILITHIUM_TRBYTES + w1_bytes;
    uint8_t *buf = (uint8_t *)malloc(len);
    memcpy(buf, mu, DILITHIUM_TRBYTES);
    for (int i = 0; i < DLT_K; i++)
        memcpy(buf + DILITHIUM_TRBYTES
                   + (size_t)i * DILITHIUM_N * sizeof(int32_t),
               w1->vec[i].coeffs,
               DILITHIUM_N * sizeof(int32_t));
    shake256_hash(c_tilde, DLT_CTILDEBYTES, buf, len);
    free(buf);
}

/* Sign (FIPS 204 Algorithm 2) */

int dilithium_sign(dilithium_sig *sig,
                   const uint8_t *msg, size_t mlen,
                   const dilithium_sk *sk)
{
    /* mu = H(tr || M) */
    uint8_t mu[DILITHIUM_TRBYTES];
    {
        size_t len = DILITHIUM_TRBYTES + mlen;
        uint8_t *buf = (uint8_t *)malloc(len);
        memcpy(buf, sk->tr, DILITHIUM_TRBYTES);
        memcpy(buf + DILITHIUM_TRBYTES, msg, mlen);
        shake256_hash(mu, DILITHIUM_TRBYTES, buf, len);
        free(buf);
    }

    /* rho'' = H(K || rnd || mu) */
    uint8_t rnd[DILITHIUM_RNDBYTES];
    {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) return -1;
        ssize_t r = read(fd, rnd, DILITHIUM_RNDBYTES);
        close(fd);
        if (r != DILITHIUM_RNDBYTES) return -1;
    }

    uint8_t rho_prime2[DILITHIUM_CRHBYTES];
    {
        uint8_t buf[DILITHIUM_SEEDBYTES + DILITHIUM_RNDBYTES
                   + DILITHIUM_TRBYTES];
        memcpy(buf, sk->K, DILITHIUM_SEEDBYTES);
        memcpy(buf + DILITHIUM_SEEDBYTES, rnd, DILITHIUM_RNDBYTES);
        memcpy(buf + DILITHIUM_SEEDBYTES + DILITHIUM_RNDBYTES,
               mu, DILITHIUM_TRBYTES);
        shake256_hash(rho_prime2, DILITHIUM_CRHBYTES, buf, sizeof(buf));
    }

    /* Expand matrix A */
    polymat A;
    expand_matrix(&A, sk->rho, DLT_K, DLT_L);

    /* Make a copy of s1, s2 that we can reduce / normalise */
    polyvecl s1_hat = sk->s1;
    polyveck s2_hat = sk->s2;
    polyveck t0_hat = sk->t0;

    /* Rejection-sampling signing loop */
    uint16_t kappa = 0;
    polyvecl y, z;
    polyveck w, w1, w0, cs2, ct0;
    poly     c_poly;

    while (1) {
        /* (a) y = ExpandMask(rho'', kappa .. kappa+l-1) */
        polyvecl_uniform_gamma1(&y, rho_prime2, kappa, DLT_L, DLT_GAMMA1);

        /* (b) w = A * y */
        polyvec_matrix_pointwise(&w, &A, &y, DLT_K, DLT_L);
        polyveck_caddq(&w, DLT_K);

        /* (b') Decompose w into (w1, w0) */
        polyveck_decompose(&w1, &w0, &w, DLT_K, DLT_GAMMA2);

        /* (c) c_tilde = H(mu || w1) */
        hash_w1(sig->c_tilde, mu, &w1);

        /* (d) c = SampleInBall(c_tilde) */
        poly_challenge(&c_poly, sig->c_tilde, DLT_TAU);

        /* (e) z = y + c*s1 */
        polyvecl_pointwise_poly(&z, &c_poly, &s1_hat, DLT_L);
        polyvecl_add(&z, &z, &y, DLT_L);
        polyvecl_reduce(&z, DLT_L);

        /* (f) Reject if ||z||_inf >= gamma1 - beta */
        if (polyvecl_chknorm(&z, DLT_L, DLT_GAMMA1 - DLT_BETA)) {
            kappa = (uint16_t)(kappa + DLT_L);
            continue;
        }

        /* (g) cs2 = c*s2; r0 = w - cs2 (low bits for check) */
        polyveck_pointwise_poly(&cs2, &c_poly, &s2_hat, DLT_K);
        polyveck_sub(&w0, &w, &cs2, DLT_K);           /* reuse w0 as tmp */
        polyveck_reduce(&w0, DLT_K);
        polyveck_caddq(&w0, DLT_K);

        {
            polyveck r0_tmp;
            polyveck_decompose(&w1, &r0_tmp, &w0, DLT_K, DLT_GAMMA2);
            /* discard the recomputed w1, we already have the right one */

            /* (h) Reject if ||r0||_inf >= gamma2 - beta */
            if (polyveck_chknorm(&r0_tmp, DLT_K, DLT_GAMMA2 - DLT_BETA)) {
                kappa = (uint16_t)(kappa + DLT_L);
                continue;
            }
        }

        /* (i) ct0 = c*t0 */
        polyveck_pointwise_poly(&ct0, &c_poly, &t0_hat, DLT_K);
        polyveck_reduce(&ct0, DLT_K);

        /* Reject if ||ct0||_inf >= gamma2 */
        if (polyveck_chknorm(&ct0, DLT_K, DLT_GAMMA2)) {
            kappa = (uint16_t)(kappa + DLT_L);
            continue;
        }

        /* w - cs2 + ct0 (argument for MakeHint) */
        polyveck wcs2ct0;
        polyveck_add(&wcs2ct0, &w0, &ct0, DLT_K);   /* w0 = w - cs2 here */
        polyveck_caddq(&wcs2ct0, DLT_K);

        /* -ct0 (first argument for MakeHint) */
        polyveck neg_ct0;
        memset(&neg_ct0, 0, sizeof(neg_ct0));
        polyveck_sub(&neg_ct0, &neg_ct0, &ct0, DLT_K);
        polyveck_reduce(&neg_ct0, DLT_K);
        polyveck_caddq(&neg_ct0, DLT_K);

        /* h = MakeHint(-ct0, w - cs2 + ct0) */
        unsigned int hint_sum;
        hint_sum = polyveck_make_hint(&sig->h, &neg_ct0, &wcs2ct0,
                                      DLT_K, DLT_GAMMA2);

        /* (j) Reject if too many hints */
        if (hint_sum > (unsigned int)DLT_OMEGA) {
            kappa = (uint16_t)(kappa + DLT_L);
            continue;
        }

        /* (k) Accept */
        sig->z = z;
        return 0;
    }
}

/* Verify (FIPS 204 Algorithm 3) */

int dilithium_verify(const dilithium_sig *sig,
                     const uint8_t *msg, size_t mlen,
                     const dilithium_pk *pk)
{
    /* Quick rejection checks */
    if (polyvecl_chknorm(&sig->z, DLT_L, DLT_GAMMA1 - DLT_BETA))
        return -1;

    /* Count hint bits; reject if > omega */
    {
        int hint_sum = 0;
        for (int i = 0; i < DLT_K; i++)
            for (int j = 0; j < DILITHIUM_N; j++)
                hint_sum += (sig->h.vec[i].coeffs[j] != 0);
        if (hint_sum > DLT_OMEGA) return -1;
    }

    /* Recompute tr, then mu */
    uint8_t tr[DILITHIUM_TRBYTES];
    recompute_tr(tr, pk);

    uint8_t mu[DILITHIUM_TRBYTES];
    {
        size_t len = DILITHIUM_TRBYTES + mlen;
        uint8_t *buf = (uint8_t *)malloc(len);
        memcpy(buf, tr, DILITHIUM_TRBYTES);
        memcpy(buf + DILITHIUM_TRBYTES, msg, mlen);
        shake256_hash(mu, DILITHIUM_TRBYTES, buf, len);
        free(buf);
    }

    /* c = SampleInBall(c_tilde) */
    poly c_poly;
    poly_challenge(&c_poly, sig->c_tilde, DLT_TAU);

    /* w' = A*z - c*t1*2^d
     * Pre-scale t1 by 2^D before multiplication by c. */
    polymat A;
    expand_matrix(&A, pk->rho, DLT_K, DLT_L);

    polyveck Az;
    polyvec_matrix_pointwise(&Az, &A, &sig->z, DLT_K, DLT_L);
    polyveck_caddq(&Az, DLT_K);

    /* t1_scaled = t1 << D */
    polyveck t1_scaled;
    for (int i = 0; i < DLT_K; i++)
        t1_scaled.vec[i] = pk->t1.vec[i];
    polyveck_shiftl(&t1_scaled, DLT_K);

    /* ct1_scaled = c * t1_scaled */
    polyveck ct1_scaled;
    polyveck_pointwise_poly(&ct1_scaled, &c_poly, &t1_scaled, DLT_K);
    polyveck_reduce(&ct1_scaled, DLT_K);
    polyveck_caddq(&ct1_scaled, DLT_K);

    /* w' = Az - ct1_scaled */
    polyveck w_prime;
    polyveck_sub(&w_prime, &Az, &ct1_scaled, DLT_K);
    polyveck_reduce(&w_prime, DLT_K);
    polyveck_caddq(&w_prime, DLT_K);

    /* w1' = UseHint(h, w') */
    polyveck w1_prime;
    polyveck_use_hint(&w1_prime, &w_prime, &sig->h, DLT_K, DLT_GAMMA2);

    /* c_tilde' = H(mu || w1') */
    uint8_t c_tilde_prime[DLT_CTILDEBYTES];
    hash_w1(c_tilde_prime, mu, &w1_prime);

    /* Accept iff c_tilde == c_tilde' */
    if (memcmp(sig->c_tilde, c_tilde_prime, DLT_CTILDEBYTES) != 0)
        return -1;

    return 0;
}
