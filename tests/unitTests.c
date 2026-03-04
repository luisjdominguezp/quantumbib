#include <criterion/criterion.h>
#include <time.h>
#include <string.h>
#include "../addition/addition.h"
#include "../subtraction/subtracion.h"
#include "../multiplication/multiplication.h"
#include "../barrett_reduction/reduction.h"
#include "../exponentiation/expo.h"
#include "../montgomery/montgomery.h"
#include "../mont_expo/mont_expo.h"
#include "../sqrt_tonelli_shanks/sqrt.h"
#include "../mod_inv/inv.h"
#include "../random/random.h"
#include "../check0s/check0s.h"
#include "../check1s/check1s.h"
#include "../hash/sha3.h"
#include "../dilithium/dilithium_poly.h"
#include "../dilithium/dilithium_keygen.h"
#include "../dilithium/dilithium_sign.h"

#define SIZE 4
#define P_SIZE 5
#define R_SIZE 8
#define BIT_LIMIT 64
#define bw 64
#define MOD_HEX "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF" 

// Copyright © 2024 Horacio Hernandez

Test(addition, add_with_carry_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
    unsigned long long p2[SIZE] = {0x1111111111111111, 0x2222222222222222, 0x3333333333333333, 0x4444444444444444};
    unsigned long long result[SIZE] = {0};

    add_with_carry(p1, p2, result, SIZE);
}


Test(subtraction, sub_with_borrow_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
    unsigned long long p2[SIZE] = {0x1111111111111111, 0x2222222222222222, 0x3333333333333333, 0x4444444444444444};
    unsigned long long result[SIZE] = {0};

    sub_with_borrow(p1, p2, result, SIZE);
}

Test(multiplication, mult_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF}; 
    unsigned long long result[R_SIZE] = {0};

    mult(p1, p2, result, SIZE, R_SIZE);
}


Test(barrett_reduction, barrett_reduction_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
    unsigned long long p2[R_SIZE] = {0x1900000000000067, 0x175700000000004d, 0xe101d68000000016, 0x2523648240000002};

    unsigned long long result[SIZE] = {0};
    reduc(p1, p2, result, SIZE, R_SIZE, bw);
}

Test(exponentiation, exponentiation_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
    unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF,0x0000000000000000, 0xFFFFFFFF00000001};

    unsigned long long result[SIZE] = {0};
    expo(p1, p2, result, SIZE, BIT_LIMIT);
}


Test(montgomery_product, montgomery_product_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
    unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF,0x0000000000000000, 0xFFFFFFFF00000001};
    mpz_t mod;
    mpz_init(mod);

    mpz_set_str(mod, MOD_HEX, 16);

    unsigned long long result[SIZE] = {0};
    montgomery_pr(p1, p2, result, SIZE, BIT_LIMIT, mod);
}


Test(montgomery_expo, montgomery_expo_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
    unsigned long long mont_expo[SIZE] = {0xFFFFFFFF00000001, 0x0000000000000000, 0x00000000FFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    mpz_t mod;
    mpz_init(mod);
    mpz_set_str(mod, MOD_HEX, 16);

    unsigned long long result[SIZE] = {0};
    montgomery_exp(p1, mont_expo, result, SIZE, BIT_LIMIT, mod);
}

Test(squareroot, squareroot_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
    unsigned long long prime2[P_SIZE] = {0xA7081AEA3BDBF56E, 0x0AE5736BE1124F8D, 0xC7CE6E75FAC521DD, 0x9F6A6B593208CDF6, 0x0E83615E354157D9};

    unsigned long long result[SIZE] = {0};
    t_sqrt(p1, prime2, result, SIZE, R_SIZE);
}

Test(modular_inverse, modular_inverse_benchmark) {
    unsigned long long modInv[SIZE] = {0x3C134124B7F5C593, 0x58BC963551FADEF0, 0x4153F9BF96E563D2, 0x3BF29687E0FE8C77};
    unsigned long long prime2[P_SIZE] = {0xA7081AEA3BDBF56E, 0x0AE5736BE1124F8D, 0xC7CE6E75FAC521DD, 0x9F6A6B593208CDF6, 0x0E83615E354157D9};

    unsigned long long result[SIZE] = {0};
    inv_mod(modInv, prime2, result, SIZE);
}

Test(check0s, check0s_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};

    int res = check0s(p1, SIZE);
}

Test(check1s, check1s_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};

    int res = check1s(p1, SIZE);
}

Test(random, random_benchmark){
    unsigned long long p1[SIZE] = {0};

    gmp_randstate_t state;
    gmp_randinit_default(state);

    unsigned long seed = (unsigned long)time(NULL);
    gmp_randseed_ui(state, seed);
    q_random(p1, state, SIZE);
}

Test(hash, hash_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
    unsigned char digest[32];
    hash_sha3_256((unsigned char *)p1, SIZE, digest);
}

/* ── Dilithium Week 6 tests ──────────────────────────────────────────────
 *
 * Test 1 – dilithium_keygen
 *   Verify that key generation runs without error and that rho, t1, t0
 *   are non-zero (i.e., the SHAKE expansion actually produced output).
 *
 * Test 2 – dilithium_sign_verify (happy path)
 *   Sign a short message with a fresh key pair and verify it.
 *   A correct implementation must accept its own signatures.
 *
 * Test 3 – dilithium_verify_tampered (reject path)
 *   Flip one byte of the message after signing.
 *   A correct implementation must reject the now-invalid signature.
 * ────────────────────────────────────────────────────────────────────── */

Test(dilithium, keygen) {
    static const uint8_t seed[32] = {
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
        0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,
        0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
        0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20
    };

    dilithium_pk pk;
    dilithium_sk sk;
    int rc = dilithium_keygen_from_seed(&pk, &sk, seed);
    cr_assert_eq(rc, 0, "keygen returned error");

    /* rho must be non-zero */
    int rho_nonzero = 0;
    for (int i = 0; i < 32; i++) rho_nonzero |= pk.rho[i];
    cr_assert_neq(rho_nonzero, 0, "rho is all-zero");

    /* t1 must have at least one non-zero coefficient */
    int t1_nonzero = 0;
    for (int i = 0; i < DLT_K && !t1_nonzero; i++)
        for (int j = 0; j < DILITHIUM_N; j++)
            t1_nonzero |= pk.t1.vec[i].coeffs[j];
    cr_assert_neq(t1_nonzero, 0, "t1 is all-zero");
}

Test(dilithium, sign_verify) {
    static const uint8_t seed[32] = {
        0xAB,0xCD,0xEF,0x01,0x23,0x45,0x67,0x89,
        0xAB,0xCD,0xEF,0x01,0x23,0x45,0x67,0x89,
        0xAB,0xCD,0xEF,0x01,0x23,0x45,0x67,0x89,
        0xAB,0xCD,0xEF,0x01,0x23,0x45,0x67,0x89
    };

    dilithium_pk pk;
    dilithium_sk sk;
    cr_assert_eq(dilithium_keygen_from_seed(&pk, &sk, seed), 0);

    const uint8_t msg[]  = "PAP II – ML-DSA Week 6 test message";
    size_t        mlen   = sizeof(msg) - 1;

    dilithium_sig sig;
    int rc_sign = dilithium_sign(&sig, msg, mlen, &sk);
    cr_assert_eq(rc_sign, 0, "sign returned error");

    int rc_verify = dilithium_verify(&sig, msg, mlen, &pk);
    cr_assert_eq(rc_verify, 0, "verify rejected a valid signature");
}

Test(dilithium, verify_tampered) {
    static const uint8_t seed[32] = {
        0xDE,0xAD,0xBE,0xEF,0xDE,0xAD,0xBE,0xEF,
        0xDE,0xAD,0xBE,0xEF,0xDE,0xAD,0xBE,0xEF,
        0xDE,0xAD,0xBE,0xEF,0xDE,0xAD,0xBE,0xEF,
        0xDE,0xAD,0xBE,0xEF,0xDE,0xAD,0xBE,0xEF
    };

    dilithium_pk pk;
    dilithium_sk sk;
    cr_assert_eq(dilithium_keygen_from_seed(&pk, &sk, seed), 0);

    uint8_t msg[]  = "Authentic message for signing";
    size_t  mlen   = sizeof(msg) - 1;

    dilithium_sig sig;
    cr_assert_eq(dilithium_sign(&sig, msg, mlen, &sk), 0);

    /* Tamper: flip a byte in the middle of the message */
    msg[4] ^= 0xFF;

    int rc = dilithium_verify(&sig, msg, mlen, &pk);
    cr_assert_neq(rc, 0, "verify accepted a tampered message");
}
