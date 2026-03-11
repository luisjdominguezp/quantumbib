#ifndef DILITHIUM_REDUCE_H
#define DILITHIUM_REDUCE_H

/*
 * Modular reduction for Dilithium.
 * Based on ref/reduce.h from https://github.com/pq-crystals/dilithium
 * Functions prefixed with dilithium_ to avoid name collisions.
 */

#include <stdint.h>
#include "dilithium_params.h"

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
int32_t dilithium_montgomery_reduce(int64_t a);

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
int32_t dilithium_reduce32(int32_t a);

/*************************************************
* Name:        dilithium_caddq
*
* Description: Add Q if input coefficient is negative.
*
* Arguments:   - int32_t a: finite field element a
*
* Returns r.
**************************************************/
int32_t dilithium_caddq(int32_t a);

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
int32_t dilithium_freeze(int32_t a);

#endif /* DILITHIUM_REDUCE_H */
