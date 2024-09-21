#include <stdio.h>
#include <inttypes.h>
#include <x86intrin.h>

#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000
#define b_w 64
#define b_b (1ULL << (b_w - 1))



void measured_function(volatile int *var) {(*var) = 1; }

__uint128_t mult128(__uint128_t num, __uint128_t num2){
    return num * num2;
}

void reduc(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]){
    //SIZE + 1
    int b_k = 5;
    //b_b^(2*b_k) / p2[0]
    __uint128_t big_b = ((__uint128_t)b_b) << (2*b_k);
    unsigned long long mu = big_b / p2[0];
    printf("Value of mu: %llu\n", mu);
    unsigned long long b_mask = 1ULL << (b_w * (b_k + 1))-1;
    printf("Value of b_mask: %llu\n", b_mask);
    unsigned long long b_expo = (b_b << (b_w * (b_k + 1)));
    printf("Value of b_expo: %llu\n", b_expo);
    unsigned long long qh[SIZE] = {0};
    unsigned long long rs[SIZE] = {0};
    
    printf("Start of 1st loop...\n");
    for(int i = 0;i<SIZE;i++){
        if (i + 1 < SIZE){
            qh[i] = (p1[i+1] * mu) >> (b_w * (b_k + 1));
            printf("Value of qh @ i: %llu - %d\n", qh[i], i);
        } else {
            qh[i] = 0;
        }
    
    }
    
    printf("Start of second loop...\n");
    for(int i = 0;i<SIZE;i++){
        //p1 % b^(n+1)
        __uint128_t mod_p1 = p1[i] & b_mask;
        printf("Value of mod_p1 @ i: %llu - %d\n", (unsigned long long)mod_p1, i);
        //(qh * p)
        __uint128_t qh_p1 = mult128(qh[i], p1[i]);
        unsigned long long qh_p1_lower = (unsigned long long)qh_p1;
        unsigned long long qh_p1_upper = (unsigned long long)(qh_p1 >> 64);
        printf("Value of qh_p1 @ i: lower=%llu, upper=%llu\n", qh_p1_lower, qh_p1_upper);
        printf("Value of qh_p1 @ i: %llu - %d\n", (unsigned long long)qh_p1, i);
        //(qh * p) % b^(n+1)
        __int128 mod_qh = qh_p1 & b_mask;
        printf("Value of mod_qh @ i: lower=%llu, upper=%llu\n", 
               (unsigned long long)(mod_qh), (unsigned long long)(mod_qh >> 64));
        if(mod_p1 >= mod_qh){
            rs[i] = mod_p1 - mod_qh;
        } else {
            rs[i] = (mod_p1 + b_b) - mod_qh;
        }
        printf("Value of rs @ i: %llu - %d\n", rs[i], i);
    }
    
    printf("Start of 3rd loop...\n");
    for(int i = 0;i<SIZE;i++){
        rs[i] = rs[i] % p2[i];
        printf("Values present in 3rd loop: rs - %llu | p2 %llu\n", rs[i], p2[i]);
    }

    printf("Final loop...\n");
    for(int i = 0;i<SIZE;i++){
        r[i] = rs[i];
    }


    

}

int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};


    unsigned long long result[R_SIZE] = {0};

    
    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    reduc(p1, p2, result);
    for(int i = 0;i<R_SIZE;i++){
        printf("%016llX\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
