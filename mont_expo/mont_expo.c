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
void montgomery_pr(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int size, int r_size, int bit_limit, mpz_t mod){
    mpz_t a, b, q, M_r, m, M_pp, t, temp, u;
    size_t count;
    mpz_inits(a, b, q, M_r, M_pp, t, temp, u, NULL);
    mpz_set_ui(M_r, 1);
    mpz_mul_2exp(M_r, M_r, size * bit_limit);

    if(mpz_invert(M_pp, m, M_r) == 0){
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
    mpz_mul(temp, q, m);
    mpz_add(temp, t, temp);

    // u = temp / R which is >> 256
    mpz_fdiv_q_2exp(u, temp, size * bit_limit);

    if (mpz_cmp(u, m) >= 0){
        mpz_sub(u, u, m);
    }

    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, u);
    mpz_clears(a, b, t, q, M_r, M_pp, temp, u, NULL);

}

void montgomery_exp(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int size, int r_size, int bit_limit, mpz_t mod){
    mpz_t base, expo, res, R, r_inv, mont_base, mont_res;
    mpz_inits(base, expo, res, R, r_inv, mont_base, mont_res, NULL);


    mpz_set_ui(R, 1);
    mpz_mul_2exp(R, R, size * bit_limit);

    if(mpz_invert(r_inv, R, mod) == 0){
        fprintf(stderr, "Error: Inverse of m does not exist.\n");
        exit(0);
    }
    mpz_import(base, size, 1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(expo, size, 1, sizeof(unsigned long long), 0, 0, p2);
    //gmp_printf("Value of base: %ZX\n", base);
    //gmp_printf("Value of expo: %ZX\n", expo);
    mpz_mul(mont_base, base, R);
    mpz_mod(mont_base, mont_base, mod);
    
    mpz_t one;
    mpz_init_set_ui(one, 1);
    mpz_mul(mont_res, one, R);
    mpz_mod(mont_res, mont_res, mod);
    mpz_clear(one);

    size_t bits = mpz_sizeinbase(expo, 2);
    for(ssize_t i = bits-1;i>=0;i--){
        unsigned long long temp1[size] = {0};
        unsigned long long temp2[size] = {0};
        mpz_export(temp1, NULL, 1, sizeof(unsigned long long), 0, 0, mont_res);
        montgomery_pr(temp1, temp1, temp2, size, r_size, bit_limit, mod);
        mpz_import(mont_res, size, 1, sizeof(unsigned long long), 0, 0, temp2);

        if(mpz_tstbit(expo, i)){
            unsigned long long mont_res_mul_arr[size] = {0};
            unsigned long long mont_base_arr[size] = {0};
            unsigned long long mul_result[size] = {0};
            mpz_export(mont_res_mul_arr, NULL, 1, sizeof(unsigned long long), 0, 0, mont_res);
            mpz_export(mont_base_arr, NULL, 1, sizeof(unsigned long long), 0, 0, mont_base);
            montgomery_pr(mont_res_mul_arr, mont_base_arr, mul_result, size, r_size, bit_limit, mod);
            mpz_import(mont_res, size, 1, sizeof(unsigned long long), 0, 0, mul_result);

        }
    }
    mpz_mul(res, mont_res, r_inv);
    mpz_mod(res, res, mod);

    size_t count;
    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, res);
    mpz_clears(base, expo, res, R, r_inv, mont_base, mont_res, NULL);
    
}

/*
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
    for(int i=0;i<NTEST;i++){
        montgomery_exp(p1, p2, result);
    }
    //montgomery_exp(p1, p2, result);
    for(int i = 0;i<R_SIZE;i++){
        printf("%016llX\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
*/
