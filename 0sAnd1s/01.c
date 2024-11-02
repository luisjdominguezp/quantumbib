#include <stdio.h>
#include <gmp.h>
#include <inttypes.h>
#include <x86intrin.h>

#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000
#define BIT_LIMIT 64

void measured_function(volatile int *var) {(*var) = 1; }
int check0s(unsigned long long p[]){
    unsigned long long mask = 1ULL;
    printf("p[3] = %llu - mask = %llu\n", p[3], mask);
    return ((p[3] & mask) == 0ULL) ? 1 : 0;
}


int check1s(unsigned long long p[]){
    unsigned long long mask = 1ULL;
    printf("p[3] = %llu - mask = %llu\n", p[3], mask);
    return((p[3] & mask) == 1ULL) ? 1 : 0;
}

int main(){
    uint64_t start, end;
    int variable = 0;


    unsigned long long p1[SIZE] = {0,0,0,0};
    unsigned long long p2[SIZE] = {0,0,0,1};


    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    for(int i=0;i<SIZE;i++){
        printf("Content of p1: %llX\n", p1[i]);
        printf("Content of p2: %llX\n", p2[i]);
    }
    printf("Content of p1 @ LSB: %d\n", p1[3]);
    printf("Content of p2 @ LSB: %d\n", p2[3]);
    int resP1 = check0s(p1);
    int resP2 = check0s(p2);
    printf("Output of check0s for p1 and p2: %d - %d\n", resP1, resP2);
    int res2P1 = check1s(p1);
    int res2P2 = check1s(p2);
    printf("Output of check1s for p1 and p2: %d - %d\n", res2P1, res2P2);
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;

}
