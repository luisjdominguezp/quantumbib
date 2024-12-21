#include <criterion/criterion.h>
#include <time.h>
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
