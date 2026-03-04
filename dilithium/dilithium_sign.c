/*
 * ML-DSA Sign and Verify  (NIST FIPS 204, Algorithms 2–3)
 * Author : Jorge Ramón Figueroa Maya  –  PAP II, Week 6
 */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include "dilithium_sign.h"

/* ── Internal: SHAKE-256 ────────────────────────────────────────────────── */

static void shake256_hash(uint8_t *out, size_t outlen,
                          const uint8_t *in, size_t inlen)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_shake256(), NULL);
    EVP_DigestUpdate(ctx, in, inlen);
    EVP_DigestFinalXOF(ctx, out, outlen);
    EVP_MD_CTX_free(ctx);
}

/* ── Internal: recompute tr = H(rho ‖ raw(t1)) from public key ──────────
 *   Must match exactly what dilithium_keygen_from_seed produces.          */
static void recompute_tr(uint8_t tr[64], const dilithium_pk *pk)
{
    size_t len = 32 + (size_t)DLT_K * DILITHIUM_N * sizeof(int32_t);
    uint8_t *buf = (uint8_t *)malloc(len);
    memcpy(buf, pk->rho, 32);
    for (int i = 0; i < DLT_K; i++)
        memcpy(buf + 32 + i * DILITHIUM_N * sizeof(int32_t),
               pk->t1.vec[i].coeffs,
               DILITHIUM_N * sizeof(int32_t));
    shake256_hash(tr, 64, buf, len);
    free(buf);
}

/* ── Internal: hash mu ‖ raw(w1) → c_tilde ────────────────────────────── */
static void hash_w1(uint8_t c_tilde[32],
                    const uint8_t mu[64],
                    const polyveck *w1)
{
    size_t w1_bytes = (size_t)DLT_K * DILITHIUM_N * sizeof(int32_t);
    size_t len = 64 + w1_bytes;
    uint8_t *buf = (uint8_t *)malloc(len);
    memcpy(buf, mu, 64);
    for (int i = 0; i < DLT_K; i++)
        memcpy(buf + 64 + i * DILITHIUM_N * sizeof(int32_t),
               w1->vec[i].coeffs,
               DILITHIUM_N * sizeof(int32_t));
    shake256_hash(c_tilde, 32, buf, len);
    free(buf);
}

/* ── Sign ────────────────────────────────────────────────────────────────── */

int dilithium_sign(dilithium_sig *sig,
                   const uint8_t *msg, size_t mlen,
                   const dilithium_sk *sk)
{
    /* ── Step 1: mu = SHAKE-256(tr ‖ M, 64) ─────────────────────────────── */
    uint8_t mu[64];
    {
        size_t len = 64 + mlen;
        uint8_t *buf = (uint8_t *)malloc(len);
        memcpy(buf, sk->tr, 64);
        memcpy(buf + 64, msg, mlen);
        shake256_hash(mu, 64, buf, len);
        free(buf);
    }

    /* ── Step 2–3: rho_prime2 = SHAKE-256(K ‖ rnd ‖ mu, 64) ─────────────── */
    uint8_t rnd[32];
    {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) return -1;
        ssize_t r = read(fd, rnd, 32);
        close(fd);
        if (r != 32) return -1;
    }

    uint8_t rho_prime2[64];
    {
        uint8_t buf[128];   /* K(32) + rnd(32) + mu(64) */
        memcpy(buf,      sk->K, 32);
        memcpy(buf + 32, rnd,   32);
        memcpy(buf + 64, mu,    64);
        shake256_hash(rho_prime2, 64, buf, 128);
    }

    /* ── Expand matrix A (needed each call; store on stack) ──────────────── */
    polymat A;
    expand_matrix(&A, sk->rho, DLT_K, DLT_L);

    /* ── Rejection-sampling signing loop ─────────────────────────────────── */
    uint16_t kappa = 0;

    while (1) {
        /* a. y = ExpandMask(rho_prime2, kappa .. kappa+l-1, gamma1) */
        polyvecl y;
        for (int i = 0; i < DLT_L; i++)
            poly_uniform_gamma1(&y.vec[i], rho_prime2,
                                (uint16_t)(kappa + i), DLT_GAMMA1);

        /* b. w = A · y */
        polyveck w;
        polyvecl_matrix_mul(&w, &A, &y, DLT_K, DLT_L);
        for (int i = 0; i < DLT_K; i++) poly_caddq(&w.vec[i]);

        /* c. w1 = HighBits(w, 2·gamma2) */
        polyveck w1;
        for (int i = 0; i < DLT_K; i++)
            poly_highbits(&w1.vec[i], &w.vec[i], DLT_GAMMA2);

        /* d. c̃ = H(mu ‖ raw(w1), 32) */
        hash_w1(sig->c_tilde, mu, &w1);

        /* e. c = SampleInBall(c̃, tau) */
        poly c;
        poly_challenge(&c, sig->c_tilde, DLT_TAU);

        /* f. z = y + c·s1 */
        polyvecl cs1;
        poly_vec_mul_scalar(cs1.vec, &c, sk->s1.vec, DLT_L);
        poly_vec_add(sig->z.vec, y.vec, cs1.vec, DLT_L);
        poly_vec_reduce(sig->z.vec, DLT_L);

        /* g. r0 = LowBits(w − c·s2, 2·gamma2) */
        polyveck cs2;
        poly_vec_mul_scalar(cs2.vec, &c, sk->s2.vec, DLT_K);

        polyveck w_minus_cs2;
        poly_vec_sub(w_minus_cs2.vec, w.vec, cs2.vec, DLT_K);
        poly_vec_reduce(w_minus_cs2.vec, DLT_K);
        poly_vec_caddq(w_minus_cs2.vec, DLT_K);

        polyveck r0;
        for (int i = 0; i < DLT_K; i++)
            poly_lowbits(&r0.vec[i], &w_minus_cs2.vec[i], DLT_GAMMA2);

        /* h. Rejection check 1 */
        if (poly_vec_inf_norm(sig->z.vec, DLT_L) >= DLT_GAMMA1 - DLT_BETA ||
            poly_vec_inf_norm(r0.vec, DLT_K)     >= DLT_GAMMA2 - DLT_BETA) {
            kappa = (uint16_t)(kappa + DLT_L);
            continue;
        }

        /* i. ct0 = c·t0;  build hint h */
        polyveck ct0;
        poly_vec_mul_scalar(ct0.vec, &c, sk->t0.vec, DLT_K);
        poly_vec_reduce(ct0.vec, DLT_K);

        /* neg_ct0[i] = -ct0[i]  (zero-init first: poly_sub writes c = a - b) */
        polyveck neg_ct0;
        memset(&neg_ct0, 0, sizeof(neg_ct0));
        for (int i = 0; i < DLT_K; i++) {
            poly_sub(&neg_ct0.vec[i], &neg_ct0.vec[i], &ct0.vec[i]);
            poly_reduce(&neg_ct0.vec[i]);
            poly_caddq(&neg_ct0.vec[i]);
        }

        /* r_hint = w − c·s2 + c·t0  (= w_minus_cs2 + ct0) */
        polyveck r_hint;
        poly_vec_add(r_hint.vec, w_minus_cs2.vec, ct0.vec, DLT_K);
        poly_vec_reduce(r_hint.vec, DLT_K);
        poly_vec_caddq(r_hint.vec, DLT_K);

        /* MakeHint(-ct0, r_hint) */
        int hint_sum = 0;
        for (int i = 0; i < DLT_K; i++)
            hint_sum += poly_makehint(&sig->h.vec[i],
                                     &neg_ct0.vec[i],
                                     &r_hint.vec[i],
                                     DLT_GAMMA2);

        /* j. Rejection check 2 */
        if (poly_vec_inf_norm(ct0.vec, DLT_K) >= DLT_GAMMA2 ||
            hint_sum > DLT_OMEGA) {
            kappa = (uint16_t)(kappa + DLT_L);
            continue;
        }

        /* k. Accept */
        return 0;
    }
}

/* ── Verify ──────────────────────────────────────────────────────────────── */

int dilithium_verify(const dilithium_sig *sig,
                     const uint8_t *msg, size_t mlen,
                     const dilithium_pk *pk)
{
    /* Quick bound check on z */
    if (poly_vec_inf_norm((const poly *)sig->z.vec, DLT_L) >=
        DLT_GAMMA1 - DLT_BETA)
        return -1;

    /* Count hint bits */
    int hint_sum = 0;
    for (int i = 0; i < DLT_K; i++)
        for (int j = 0; j < DILITHIUM_N; j++)
            hint_sum += sig->h.vec[i].coeffs[j];
    if (hint_sum > DLT_OMEGA) return -1;

    /* ── Step 1: recompute tr, then mu ──────────────────────────────────── */
    uint8_t tr[64];
    recompute_tr(tr, pk);

    uint8_t mu[64];
    {
        size_t len = 64 + mlen;
        uint8_t *buf = (uint8_t *)malloc(len);
        memcpy(buf, tr, 64);
        memcpy(buf + 64, msg, mlen);
        shake256_hash(mu, 64, buf, len);
        free(buf);
    }

    /* ── Step 3: c = SampleInBall(c̃, tau) ─────────────────────────────── */
    poly c;
    poly_challenge(&c, sig->c_tilde, DLT_TAU);

    /* ── Step 4: w_prime = A·z − c·(t1·2^d) ─────────────────────────────
     *   We compute in two parts:                                            *
     *     Az  = A · z                                                      *
     *     ct1 = c · t1,  then scale by 2^d (left-shift each coeff by d)   */
    polymat A;
    expand_matrix(&A, pk->rho, DLT_K, DLT_L);

    polyveck Az;
    polyvecl_matrix_mul(&Az, &A, (const polyvecl *)&sig->z, DLT_K, DLT_L);
    for (int i = 0; i < DLT_K; i++) poly_caddq(&Az.vec[i]);

    /* Scale t1 by 2^d BEFORE multiplying by c.
     * t1 coeffs ∈ [0, (q-1)/2^d] = [0, 1023]; after << 13 they stay in
     * [0, 1023 * 8192] = [0, 8 380 416] < q  ⟹  no int32_t overflow.
     * Doing the shift AFTER the schoolbook mul would overflow (values up to
     * (q-1) * 2^13 ≈ 68 billion >> INT32_MAX). */
    polyveck t1_scaled;
    for (int i = 0; i < DLT_K; i++)
        for (int j = 0; j < DILITHIUM_N; j++)
            t1_scaled.vec[i].coeffs[j] = pk->t1.vec[i].coeffs[j] << DILITHIUM_D;

    polyveck ct1_scaled;
    for (int i = 0; i < DLT_K; i++) {
        poly_schoolbook_mul(&ct1_scaled.vec[i], &c, &t1_scaled.vec[i]);
        poly_reduce(&ct1_scaled.vec[i]);
        poly_caddq(&ct1_scaled.vec[i]);
    }

    polyveck w_prime;
    poly_vec_sub(w_prime.vec, Az.vec, ct1_scaled.vec, DLT_K);
    poly_vec_reduce(w_prime.vec, DLT_K);
    poly_vec_caddq(w_prime.vec, DLT_K);

    /* ── Step 5: w1_prime = UseHint(h, w_prime, 2·gamma2) ──────────────── */
    polyveck w1_prime;
    for (int i = 0; i < DLT_K; i++)
        poly_usehint(&w1_prime.vec[i], &sig->h.vec[i],
                     &w_prime.vec[i], DLT_GAMMA2);

    /* ── Step 6: c̃' = H(mu ‖ raw(w1_prime), 32) ──────────────────────── */
    uint8_t c_tilde_prime[32];
    hash_w1(c_tilde_prime, mu, &w1_prime);

    /* ── Step 7: accept iff c̃ = c̃' ─────────────────────────────────────── */
    if (memcmp(sig->c_tilde, c_tilde_prime, 32) != 0) return -1;

    return 0;
}
