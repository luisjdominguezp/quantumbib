#include <stdio.h>
#include <gmp.h>
#include <inttypes.h>
#include <x86intrin.h>
/*
#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000
#define BIT_LIMIT 64
#define MOD_HEX "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF" 

void measured_function(volatile int *var) {(*var) = 1; }
*/
// Copyright © 2024 Horacio Hernandez
void montgomery_pr(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int size, int bit_limit, mpz_t mod_hex){
    // m * m'  = -1 mod R
    // aR mod R
    //Montgomery(a, b) = a*b*R
    // t = a * b
    // m = (t * m') mod R
    // u = (t + m * m)/R
    // if u >= m -> u = u - m
    // return u
    mpz_t a, b, q, M_r, M_pp, t, temp, u;
    size_t count;
    mpz_inits(a, b, q, M_r, M_pp, t, temp, u, NULL);
    //mpz_init_set_str(m, mod_hex, 16);
    mpz_set_ui(M_r, 1);
    mpz_mul_2exp(M_r, M_r, size * bit_limit);

    if(mpz_invert(M_pp, mod_hex, M_r) == 0){
        fprintf(stderr, "Error: Inverse of m does not exist.\n");
        exit(0);
    }
    mpz_neg(M_pp, M_pp);
    mpz_mod(M_pp, M_pp, M_r);

    mpz_import(a, size, 1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(b, size, 1, sizeof(unsigned long long), 0, 0, p2);
    
    mpz_mul(t, a, b);
    mpz_mul(q, t, M_pp);
    //mod R
    mpz_tdiv_r_2exp(q, q, size * bit_limit);
    mpz_mul(temp, q, mod_hex);
    mpz_add(temp, t, temp);

    // u = temp / R which is >> 256
    mpz_fdiv_q_2exp(u, temp, size * bit_limit);

    if (mpz_cmp(u, mod_hex) >= 0){
        mpz_sub(u, u, mod_hex);
    }

    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, u);
    mpz_clears(a, b, t, q, M_r, M_pp, temp, u, NULL);

}
/*
int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    //unsigned long long p1[SIZE] = {2, 0, 0, 0};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF,0x0000000000000000, 0xFFFFFFFF00000001};
    //unsigned long long p2[SIZE] = {0x1900000000000067, 0x175700000000004d, 0xe101d68000000016, 0x2523648240000002};
    
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0, 0, 0, 0};
    unsigned long long p2[SIZE] = {2, 0, 0, 0};
    //unsigned long long p2[SIZE] = {5, 0, 0, 0};

    unsigned long long result[R_SIZE] = {0};


    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    for(int i=0;i<NTEST;i++){
        montgomery_pr(p1, p2, result);
    }
    for(int i = 0;i<R_SIZE;i++){
        printf("%016llX\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}

*/
