#ifndef DILITHIUM_KEYGEN_H
#define DILITHIUM_KEYGEN_H

/*
 * Dilithium / ML-DSA key generation.
 * Based on ref/sign.c (crypto_sign_keypair) from
 * https://github.com/pq-crystals/dilithium
 *
 * Keys kept in unpacked structs; SHAKE via OpenSSL; schoolbook mul.
 * Parameter set selected via DLT_* macros below (defaults to ML-DSA-44).
 */

#include <stdint.h>
#include "dilithium_params.h"
#include "dilithium_poly.h"

/* Active parameter set (ML-DSA-44 by default) */
#define DLT_K       DILITHIUM2_K
#define DLT_L       DILITHIUM2_L
#define DLT_ETA     DILITHIUM2_ETA
#define DLT_TAU     DILITHIUM2_TAU
#define DLT_BETA    DILITHIUM2_BETA
#define DLT_GAMMA1  DILITHIUM2_GAMMA1
#define DLT_GAMMA2  DILITHIUM2_GAMMA2
#define DLT_OMEGA   DILITHIUM2_OMEGA
#define DLT_CTILDEBYTES DILITHIUM2_CTILDEBYTES

/* Key structures */

typedef struct {
  uint8_t  rho[DILITHIUM_SEEDBYTES]; /* Public seed for matrix A          */
  polyveck t1;                       /* High bits of t  (k polynomials)   */
  polyveck t;                        /* Full t = A*s1 + s2  (convenience) */
} dilithium_pk;

typedef struct {
  uint8_t  rho[DILITHIUM_SEEDBYTES]; /* Public seed (mirrored from pk)    */
  uint8_t  rho_prime[DILITHIUM_CRHBYTES]; /* Private seed for s1,s2,y    */
  uint8_t  K[DILITHIUM_SEEDBYTES];   /* Signing randomness input          */
  uint8_t  tr[DILITHIUM_TRBYTES];    /* Public-key hash H(rho || t1)      */
  polyvecl s1;                       /* Secret vector (l small polys)     */
  polyveck s2;                       /* Error vector  (k small polys)     */
  polyveck t0;                       /* Low bits of t                     */
  polyveck t1;                       /* High bits of t (same as pk.t1)    */
} dilithium_sk;

/* API */

/*************************************************
* Name:        dilithium_keygen
*
* Description: Generates public and private key.
*              Reads 32 random bytes from /dev/urandom.
*
* Arguments:   - dilithium_pk *pk: pointer to output public key
*              - dilithium_sk *sk: pointer to output private key
*
* Returns 0 on success, -1 if /dev/urandom is unavailable.
**************************************************/
int dilithium_keygen(dilithium_pk *pk, dilithium_sk *sk);

/*************************************************
* Name:        dilithium_keygen_from_seed
*
* Description: Deterministic key generation from caller-supplied seed.
*              Useful for testing / reproducibility.
*
* Arguments:   - dilithium_pk *pk: pointer to output public key
*              - dilithium_sk *sk: pointer to output private key
*              - const uint8_t seed[32]: 32-byte random seed
*
* Returns 0 (success).
**************************************************/
int dilithium_keygen_from_seed(dilithium_pk *pk, dilithium_sk *sk,
                               const uint8_t seed[DILITHIUM_SEEDBYTES]);

#endif /* DILITHIUM_KEYGEN_H */
