#include <stdio.h>
#include <gmp.h>
#include <inttypes.h>
#include <x86intrin.h>

#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000


void measured_function(volatile int *var) {(*var) = 1; }

void expo(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]){
    mpz_t base, expo, res;
    size_t count;
    mpz_inits(base, expo, res, NULL);
    mpz_import(base, SIZE, -1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(expo, SIZE, -1, sizeof(unsigned long long), 0, 0, p2);
    gmp_printf("Value of base: %Zd\n", base);
    gmp_printf("Value of expo: %Zd\n", expo);
    //check to see if expo is 0, 1 or 2
    if(mpz_cmp_ui(expo, 0) == 0){
        mpz_set_ui(res, 1);
        mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, res);
        mpz_clears(base, expo, res, NULL);
        return;
    }
    if(mpz_cmp_ui(expo, 1) == 0){
        mpz_set(res, base);
        mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, res);
        mpz_clears(base, expo, res, NULL);
        return;
    }
    if(mpz_cmp_ui(expo, 2) == 0){
        mpz_mul(res, base, base);
        mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, res);
        mpz_clears(base, expo, res, NULL);
        return;
    }

    size_t bits = mpz_sizeinbase(expo, 2);
    if(bits == 0){
        bits = 1;
    }
    //g = f
    mpz_set(res, base);
    for(ssize_t i=bits-2;i>=0;i--){
        //square
        mpz_mul(res, res, res);
        if(mpz_tstbit(expo, i)){
            mpz_mul(res, res, base);
        }
    }

    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, res);
    mpz_clears(base, expo, res, NULL);
}

int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    unsigned long long p2[SIZE] = {2, 0, 0, 0};
    //unsigned long long p1[SIZE] = {2, 0, 0, 0};
    //unsigned long long p2[SIZE] = {5, 0, 0, 0};

    unsigned long long result[R_SIZE] = {0};


    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    expo(p1, p2, result);
    for(int i = 0;i<R_SIZE;i++){
        printf("%016llX\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
