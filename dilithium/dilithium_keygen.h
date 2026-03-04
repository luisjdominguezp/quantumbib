#ifndef DILITHIUM_KEYGEN_H
#define DILITHIUM_KEYGEN_H

/*
 * ML-DSA Key Generation
 * Standard : NIST FIPS 204, Algorithm 1 (ML-DSA.KeyGen_internal)
 * Author   : Jorge Ramón Figueroa Maya  –  PAP II, Week 6
 *
 * ── Algorithm (simplified, fixed to ML-DSA-44) ──────────────────────────
 *
 *   Input : 32-byte random seed zeta  (from /dev/urandom)
 *   Output: pk = (rho, t1)
 *           sk = (rho, rho_prime, K, tr, s1, s2, t0)
 *
 *   1.  (rho ‖ rho_prime ‖ K)  =  SHAKE-256(zeta ‖ k ‖ l,  128 bytes)
 *   2.  A  =  ExpandA(rho)              – k×l uniform matrix
 *   3.  s1 =  ExpandS(rho_prime, 0..l-1, eta)   – small l-vector
 *       s2 =  ExpandS(rho_prime, l..l+k-1, eta) – small k-vector
 *   4.  t  =  A·s1 + s2                – in coefficient domain
 *   5.  (t1, t0) = Power2Round(t, d=13)
 *   6.  pk  = (rho, t1)
 *   7.  tr  = SHAKE-256(rho ‖ raw(t1), 64)
 *       sk  = (rho, rho_prime, K, tr, s1, s2, t0)
 *
 * ── Parameter set: ML-DSA-44 ────────────────────────────────────────────
 *   k=4, l=4, eta=2, tau=39, gamma1=2^17, gamma2=95232, beta=78, omega=80
 */

#include <stdint.h>
#include "dilithium_params.h"
#include "dilithium_poly.h"

/* Fixed parameter set for this implementation */
#define DLT_K       DILITHIUM2_K
#define DLT_L       DILITHIUM2_L
#define DLT_ETA     DILITHIUM2_ETA
#define DLT_TAU     DILITHIUM2_TAU
#define DLT_BETA    DILITHIUM2_BETA
#define DLT_GAMMA1  DILITHIUM2_GAMMA1
#define DLT_GAMMA2  DILITHIUM2_GAMMA2
#define DLT_OMEGA   DILITHIUM2_OMEGA

/* ── Key structures ─────────────────────────────────────────────────────── */

typedef struct {
    uint8_t  rho[32];      /* Public seed for matrix A               */
    polyveck t1;           /* High bits of t  (k polynomials)        */
    polyveck t;            /* Full t = A·s1 + s2  (for convenience)  */
} dilithium_pk;

typedef struct {
    uint8_t  rho[32];      /* Public seed (mirrored from pk)         */
    uint8_t  rho_prime[64];/* Private seed for s1, s2, y             */
    uint8_t  K[32];        /* Signing randomness input               */
    uint8_t  tr[64];       /* Public-key hash H(rho ‖ t1)            */
    polyvecl s1;           /* Secret vector (l small polynomials)    */
    polyveck s2;           /* Error vector  (k small polynomials)    */
    polyveck t0;           /* Low bits of t                          */
    polyveck t1;           /* High bits of t  (same as pk.t1)        */
} dilithium_sk;

/* ── API ────────────────────────────────────────────────────────────────── */

/*
 * dilithium_keygen – Generate a fresh ML-DSA-44 key pair.
 *
 * Reads 32 random bytes from /dev/urandom.
 * Returns 0 on success, -1 if /dev/urandom is unavailable.
 */
int dilithium_keygen(dilithium_pk *pk, dilithium_sk *sk);

/*
 * dilithium_keygen_from_seed – Deterministic key generation (testing only).
 *
 * seed : caller-supplied 32 bytes; MUST be uniformly random in production.
 */
int dilithium_keygen_from_seed(dilithium_pk *pk, dilithium_sk *sk,
                               const uint8_t seed[32]);

#endif /* DILITHIUM_KEYGEN_H */
