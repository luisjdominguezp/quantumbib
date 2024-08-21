#include <stdio.h>
#include <inttypes.h>
#include <x86intrin.h>

#define SIZE 4
#pragma intrinsic(__rdtsc)
#define NTEST 100000


void measured_function(volatile int *var) {(*var) = 1; }

void sub_with_borrow(long long p1[], long long p2[], long long r[]){
    long long borrow = 0;
    for(int i = 0;i<SIZE;i++){
        //difference
        r[i] = p1[i] - p2[i] - borrow;
        borrow = (p1[i] < p2[i] + borrow);
    }
}

int main(){
    uint64_t start, end;
    int variable = 0;

    long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //long long p1[SIZE] = {2, 4, 6, 8};
    //long long p2[SIZE] = {1, 2, 3, 4};


    long long result[SIZE] = {0};
    
    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    sub_with_borrow(p1, p2, result);
    for(int i = 0;i<SIZE;i++){
        printf("%X\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
