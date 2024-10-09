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

void montgomery_pr(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]){
    // m * m'  = -1 mod R
    // aR mod R
    //Montgomery(a, b) = a*b*R
    // t = a * b
    // m = (t * m') mod R
    // u = (t + m * m)/R
    // if u >= m -> u = u - m
    // return u
    mpz_t a, b, q, M_r, m, M_pp, t, temp, u;
    size_t count;
    mpz_inits(a, b, q, M_r, m, M_pp, t, temp, u, NULL);
    mpz_init_set_str(m, MOD_HEX, 16);
    mpz_set_ui(M_r, 1);
    mpz_mul_2exp(M_r, M_r, SIZE * BIT_LIMIT);

    if(mpz_invert(M_pp, m, M_r) == 0){
        fprintf(stderr, "Error: Inverse of m does not exist.\n");
        exit(0);
    }
    mpz_neg(M_pp, M_pp);
    mpz_mod(M_pp, M_pp, M_r);

    mpz_import(a, SIZE, 1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(b, SIZE, 1, sizeof(unsigned long long), 0, 0, p2);
    
    mpz_mul(t, a, b);
    mpz_mul(q, t, M_pp);
    //mod R
    mpz_tdiv_r_2exp(q, q, SIZE * BIT_LIMIT);
    mpz_mul(temp, q, m);
    mpz_add(temp, t, temp);

    // u = temp / R which is >> 256
    mpz_fdiv_q_2exp(u, temp, SIZE * BIT_LIMIT);

    //SET VALUES OF m and prime to that of values in params.py
    if (mpz_cmp(u, m) >= 0){
        mpz_sub(u, u, m);
    }

    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, u);
    mpz_clears(a, b, t, q, M_r, M_pp, m, temp, u, NULL);

}

int main(){
    uint64_t start, end;
    int variable = 0;

    unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    //unsigned long long p1[SIZE] = {2, 0, 0, 0};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF,0x0000000000000000, 0xFFFFFFFF00000001};
    //unsigned long long p2[SIZE] = {0x1900000000000067, 0x175700000000004d, 0xe101d68000000016, 0x2523648240000002};
    
    //unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0, 0, 0, 0};
    //unsigned long long p1[SIZE] = {2, 0, 0, 0};
    //unsigned long long p2[SIZE] = {5, 0, 0, 0};

    unsigned long long result[SIZE] = {0};


    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    montgomery_pr(p1, p2, result);
    for(int i = 0;i<SIZE;i++){
        printf("%016llX\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}