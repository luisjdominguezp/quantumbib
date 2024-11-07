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

int check1s(unsigned long long p[]){
    unsigned long long mask = 1ULL;
    //printf("p[SIZE-1] = %llX - mask = %llX\n", p[SIZE-1], mask);
    return((p[SIZE-1] & mask) == 1ULL) ? 1 : 0;
}

int main(){
    uint64_t start, end;
    int variable = 0;


    //unsigned long long p1[SIZE] = {0,0,0,0};
    //unsigned long long p2[SIZE] = {0,0,0,1};

    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE};
    unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};

    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    int res1 = 0;
    int res2 = 0;
    for(int i=0;i<NTEST;i++){    
        int resP1 = check1s(p1);
        int resP2 = check1s(p2);
        res1 = resP1;
        res2 = resP2;
    }
    /*for(int i=0;i<SIZE;i++){
        printf("Content of p1: %llX\n", p1[i]);
    }
    */
    /*printf("---------\n");
    for(int i=0;i<SIZE;i++){
        printf("Content of p2: %llX\n", p2[i]);
    }
    printf("Content of p1 @ LSB: %016llX\n", p1[SIZE-1]);
    printf("Content of p2 @ LSB: %016llX\n", p2[SIZE-1]);
    */
    //int resP1 = check0s(p1);
    //int resP2 = check0s(p2);
    //printf("Output of check0s for p1 and p2: %d - %d\n", resP1, resP2);
    //int res2P1 = check1s(p1);
    //int res2P2 = check1s(p2);
    //printf("Output of check1s for p1 and p2: %d - %d\n", res2P1, res2P2);
    end = __rdtsc();
    printf("Result for check1s: %d - %d\n", res1, res2); 
    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;

}
