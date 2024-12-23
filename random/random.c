#include <stdio.h>
#include <time.h>
#include <gmp.h>
#include <inttypes.h>
#include <x86intrin.h>
/*
#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000
*/
/*
void measured_function(volatile int *var) {(*var) = 1; }
*/
// Copyright © 2024 Horacio Hernandez
void q_random(unsigned long long p[], gmp_randstate_t state, int size) {
    mpz_t rand_num;
    mpz_init(rand_num);

    mpz_urandomb(rand_num, state, 64 * size);
    
    //gmp_printf("Random number generated: %ZX\n", rand_num);
    size_t count;
    mpz_export(p, &count, 1, sizeof(unsigned long long), 0, 0, rand_num);

    mpz_clear(rand_num);
}
/*
int main(){
    uint64_t start, end;
    int variable = 0;


    unsigned long long p1[SIZE] = {0};
    unsigned long long p2[SIZE] = {0};

    //unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};

    gmp_randstate_t state;
    gmp_randinit_default(state);

    unsigned long long seed = (unsigned long)time(NULL);
    gmp_randseed_ui(state, seed);

    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    for(int i =0;i<NTEST;i++){
        q_random(p1, state, SIZE);
    }
    //q_random(p1, state);
    q_random(p2, state, SIZE);
    gmp_randclear(state);
    for(int i =0;i<SIZE;i++){    
        printf("Content of p1: %016llX\n", p1[i]);
    }
    printf("---------\n");
    for(int i =0;i<SIZE;i++){    
        printf("Content of p2: %016llX\n", p2[i]);
    }

    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;

}
*/
