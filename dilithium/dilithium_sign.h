#ifndef DILITHIUM_SIGN_H
#define DILITHIUM_SIGN_H

/*
 * Dilithium / ML-DSA sign and verify.
 * Based on ref/sign.c from https://github.com/pq-crystals/dilithium
 *
 * Signatures kept in unpacked structs; SHAKE via OpenSSL; schoolbook mul.
 */

#include <stdint.h>
#include <stddef.h>
#include "dilithium_poly.h"
#include "dilithium_keygen.h"

/* Signature structure */

typedef struct {
  uint8_t  c_tilde[DLT_CTILDEBYTES]; /* Challenge hash */
  polyvecl z;                        /* Response vector (l polynomials)   */
  polyveck h;                        /* Hint vector     (k polys, 0/1)   */
} dilithium_sig;

/* API */

/*************************************************
* Name:        dilithium_sign
*
* Description: Compute signed message (hedged signing).
*
* Arguments:   - dilithium_sig *sig: pointer to output signature
*              - const uint8_t *msg: pointer to message to be signed
*              - size_t mlen: length of message
*              - const dilithium_sk *sk: pointer to secret key
*
* Returns 0 on success, -1 if /dev/urandom is unavailable.
**************************************************/
int dilithium_sign(dilithium_sig *sig,
                   const uint8_t *msg, size_t mlen,
                   const dilithium_sk *sk);

/*************************************************
* Name:        dilithium_verify
*
* Description: Verify signature.
*
* Arguments:   - const dilithium_sig *sig: pointer to input signature
*              - const uint8_t *msg: pointer to message
*              - size_t mlen: length of message
*              - const dilithium_pk *pk: pointer to public key
*
* Returns 0 if signature is valid, -1 otherwise.
**************************************************/
int dilithium_verify(const dilithium_sig *sig,
                     const uint8_t *msg, size_t mlen,
                     const dilithium_pk *pk);

#endif /* DILITHIUM_SIGN_H */
