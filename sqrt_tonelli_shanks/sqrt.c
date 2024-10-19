#include <stdio.h>
#include <gmp.h>
#include <inttypes.h>
#include <x86intrin.h>

#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000
#define BIT_LIMIT 64
#define MOD_HEX "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF" 

void measured_function(volatile int *var) {(*var) = 1; }

int legendre(mpz_t a, mpz_t p){
    int k = 1;
    mpz_t A, B, temp;
    mpz_init_set(A, a);
    mpz_init_set(B, p);
    mpz_init(temp);

    while (mpz_cmp_ui(B, 1) != 0) {
        if (mpz_cmp_ui(A, 0) == 0) {
            mpz_clears(A, B, temp, NULL);
            return 0;
        }

        int v = 0;
        // While A is even
        while (mpz_even_p(A)) {
            v += 1;
            mpz_fdiv_q_2exp(A, A, 1); // A = A / 2
        }

        // t0 = B mod 8
        unsigned long t0 = mpz_fdiv_ui(B, 8);

        if ((v % 2 == 1) && (t0 == 3 || t0 == 5)) {
            k = -k;
        }

        // A_mod_4 = A mod 4
        unsigned long A_mod_4 = mpz_fdiv_ui(A, 4);
        // B_mod_4 = B mod 4
        unsigned long B_mod_4 = mpz_fdiv_ui(B, 4);

        if ((A_mod_4 == 3) && (B_mod_4 == 3)) {
            k = -k;
        }

        // r = A
        mpz_t r;
        mpz_init_set(r, A);
        // A = B % r
        mpz_mod(A, B, r);
        // B = r
        mpz_set(B, r);

        mpz_clear(r);
    }

    if (mpz_cmp_ui(B, 1) > 0) {
        k = 0;
    }

    mpz_clears(A, B, temp, NULL);

    return k;
}

void sqrt(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]){
   mpz_t f, p, result, m;
    mpz_inits(f, p, result, m, NULL);

    mpz_set_str(m, MOD_HEX, 16);

    mpz_import(f, SIZE, 1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(p, SIZE, 1, sizeof(unsigned long long), 0, 0, p2);

    mpz_set(p, m);

    int legendre_value = legendre(f, p);
    if (legendre_value != 1) {
        // No square root exists
        // Set r[0] = -1 to indicate an error
        for (int i = 0; i < R_SIZE; i++) {
            r[i] = 0;
        }
        r[0] = (unsigned long long)-1;
        mpz_clears(f, p, result, m, NULL);
        return;
    }

    mpz_t g, a, temp;
    mpz_inits(g, a, temp, NULL);

    mpz_t expo;
    mpz_init(expo);
    mpz_sub_ui(expo, p, 3);
    mpz_fdiv_q_ui(expo, expo, 4);

    //g = f^exponent mod p
    mpz_powm(g, f, expo, p);

    //temp = g^2 mod p
    mpz_mul(temp, g, g);
    mpz_mod(temp, temp, p);

    //a = temp * f mod p
    mpz_mul(a, temp, f);
    mpz_mod(a, a, p);

    //g = g * f mod p
    mpz_mul(g, g, f);
    mpz_mod(g, g, p);

    //if a == p - 1
    mpz_t p_minus_1;
    mpz_init(p_minus_1);
    mpz_sub_ui(p_minus_1, p, 1);

    if (mpz_cmp(a, p_minus_1) == 0) {
        // No square root exists
        // Set r[0] = -1 to indicate an error
        for (int i = 0; i < R_SIZE; i++) {
            r[i] = 0;
        }
        r[0] = (unsigned long long)-1;
        mpz_clears(f, p, result, m, g, a, temp, expo, p_minus_1, NULL);
        return;
    }

    mpz_set(result, g);

    size_t count;
    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, result);

    mpz_clears(f, p, result, m, g, a, temp, expo, p_minus_1, NULL); 
}

int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    unsigned long long p1[SIZE] = {0, 0, 0, 2};
    //FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF
    unsigned long long p2[SIZE] = {0xFFFFFFFF00000001, 0x0000000000000000, 0x00000000FFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF, 0x0000000000000000, 0xFFFFFFFF00000001};
    //unsigned long long p2[SIZE] = {0x1900000000000067, 0x175700000000004d, 0xe101d68000000016, 0x2523648240000002};
    
    //unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0, 0, 0, 0};
    //unsigned long long p2[SIZE] = {2, 0, 0, 0};
    //unsigned long long p2[SIZE] = {0, 0, 0, 5};

    unsigned long long result[R_SIZE] = {0};


    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    sqrt(p1, p2, result);
    if (result[0] == (unsigned long long)-1){
        printf("No square root exists.\n");
    } else {
        for(int i = 0;i<R_SIZE;i++){
        printf("%016lld\n", result[i]);
        }
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
