#include <stdint.h>
#include "dilithium_params.h"
#include "dilithium_reduce.h"

/*
 * Modular reduction for Dilithium.
 * Based on ref/reduce.c from https://github.com/pq-crystals/dilithium
 */

/*************************************************
* Name:        dilithium_montgomery_reduce
*
* Description: For finite field element a with -2^{31}Q <= a <= Q*2^{31},
*              compute r \equiv a*2^{-32} (mod Q) such that -Q < r < Q.
*
* Arguments:   - int64_t a: finite field element a
*
* Returns r.
**************************************************/
int32_t dilithium_montgomery_reduce(int64_t a) {
  int32_t t;

  t = (int64_t)(int32_t)a*DILITHIUM_QINV;
  t = (a - (int64_t)t*DILITHIUM_Q) >> 32;
  return t;
}

/*************************************************
* Name:        dilithium_reduce32
*
* Description: For finite field element a with a <= 2^{31} - 2^{22} - 1,
*              compute r \equiv a (mod Q) such that -6283008 <= r <= 6283008.
*
* Arguments:   - int32_t a: finite field element a
*
* Returns r.
**************************************************/
int32_t dilithium_reduce32(int32_t a) {
  int32_t t;

  t = (a + (1 << 22)) >> 23;
  t = a - t*DILITHIUM_Q;
  return t;
}

/*************************************************
* Name:        dilithium_caddq
*
* Description: Add Q if input coefficient is negative.
*
* Arguments:   - int32_t a: finite field element a
*
* Returns r.
**************************************************/
int32_t dilithium_caddq(int32_t a) {
  a += (a >> 31) & DILITHIUM_Q;
  return a;
}

/*************************************************
* Name:        dilithium_freeze
*
* Description: For finite field element a, compute standard
*              representative r = a mod^+ Q.
*
* Arguments:   - int32_t a: finite field element a
*
* Returns r.
**************************************************/
int32_t dilithium_freeze(int32_t a) {
  a = dilithium_reduce32(a);
  a = dilithium_caddq(a);
  return a;
}
