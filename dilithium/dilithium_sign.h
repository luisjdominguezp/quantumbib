#ifndef DILITHIUM_SIGN_H
#define DILITHIUM_SIGN_H

/*
 * ML-DSA Sign and Verify
 * Standard : NIST FIPS 204, Algorithms 2–3
 * Author   : Jorge Ramón Figueroa Maya  –  PAP II, Week 6
 *
 * ── Sign (Algorithm 2, ML-DSA.Sign_internal) ────────────────────────────
 *
 *   Input : sk, message M
 *   Output: signature σ = (c̃, z, h)
 *
 *   1.  mu         = SHAKE-256(tr ‖ M, 64)
 *   2.  rnd        = random 32 bytes
 *   3.  rho_prime2 = SHAKE-256(K ‖ rnd ‖ mu, 64)
 *   4.  kappa = 0
 *   5.  Loop:
 *       a.  y[i]  = ExpandMask(rho_prime2, kappa+i, gamma1)   i=0..l-1
 *       b.  w     = A · y
 *       c.  w1    = HighBits(w, 2·gamma2)  per coefficient
 *       d.  c̃    = SHAKE-256(mu ‖ raw(w1), 32)
 *       e.  c     = SampleInBall(c̃, tau)
 *       f.  z     = y + c·s1
 *       g.  r0    = LowBits(w − c·s2, 2·gamma2)
 *       h.  If ‖z‖∞ ≥ gamma1−beta  OR  ‖r0‖∞ ≥ gamma2−beta:
 *               kappa += l; continue
 *       i.  h     = MakeHint(−c·t0, w − c·s2 + c·t0, 2·gamma2)
 *       j.  If ‖c·t0‖∞ ≥ gamma2  OR  Σ h > omega:
 *               kappa += l; continue
 *       k.  Return σ = (c̃, z, h)
 *
 * ── Verify (Algorithm 3, ML-DSA.Verify_internal) ────────────────────────
 *
 *   Input : pk, M, σ = (c̃, z, h)
 *   Output: accept / reject
 *
 *   1.  tr        = SHAKE-256(rho ‖ raw(t1), 64)   [recomputed from pk]
 *   2.  mu        = SHAKE-256(tr ‖ M, 64)
 *   3.  c         = SampleInBall(c̃, tau)
 *   4.  w_prime   = A·z − c·(t1·2^d)
 *   5.  w1_prime  = UseHint(h, w_prime, 2·gamma2)
 *   6.  c̃_prime  = SHAKE-256(mu ‖ raw(w1_prime), 32)
 *   7.  Accept iff  c̃ = c̃_prime  AND  ‖z‖∞ < gamma1−beta  AND  Σh ≤ omega
 */

#include <stdint.h>
#include <stddef.h>
#include "dilithium_poly.h"
#include "dilithium_keygen.h"

/* ── Signature structure ────────────────────────────────────────────────── */

typedef struct {
    uint8_t  c_tilde[32];  /* 32-byte challenge hash                  */
    polyvecl z;            /* response vector  (l polynomials)        */
    polyveck h;            /* hint vector      (k polynomials, 0/1)   */
} dilithium_sig;

/* ── API ────────────────────────────────────────────────────────────────── */

/*
 * dilithium_sign – Sign a message with ML-DSA-44.
 *
 * sig  : output signature
 * msg  : message buffer
 * mlen : message length in bytes
 * sk   : secret key (from dilithium_keygen)
 * Returns 0 on success, -1 if /dev/urandom is unavailable.
 */
int dilithium_sign(dilithium_sig *sig,
                   const uint8_t *msg, size_t mlen,
                   const dilithium_sk *sk);

/*
 * dilithium_verify – Verify an ML-DSA-44 signature.
 *
 * sig  : signature to verify
 * msg  : message buffer
 * mlen : message length in bytes
 * pk   : public key (from dilithium_keygen)
 * Returns 0 if valid, -1 if invalid.
 */
int dilithium_verify(const dilithium_sig *sig,
                     const uint8_t *msg, size_t mlen,
                     const dilithium_pk *pk);

#endif /* DILITHIUM_SIGN_H */
