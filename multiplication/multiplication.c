#include <stdio.h>
#include <inttypes.h>
#include <x86intrin.h>

#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000


void measured_function(volatile int *var) {(*var) = 1; }

void mult(long long p1[], long long p2[], long long r[]){
        /* 
        unsigned long long tmpSum = p2[i] + carry;
        r[i] = p1[i] + tmpSum;
        carry = (p1[i] > r[i]) | (p2[i] > tmpSum);
        */
    //unsigned long long carry, sum;
    //unsigned long long temp;
    for(int i = 0;i<SIZE;i++){
        uint64_t carry = 0;
        for(int j = 0;j<SIZE;j++){
            // (Carry,Sum) <- r[i+j] + a[j] * b[i] + Carry
            __uint128_t temp = r[i+j] + p1[j] * p2[i] + carry;
            //sum = temp;
            r[i+j] = temp;
            carry = temp >> 64;
        }
        r[i+SIZE] += carry;
    }
}

int main(){
    uint64_t start, end;
    int variable = 0;

    //long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    long long p1[SIZE] = {2, 4, 6, 8};
    long long p2[SIZE] = {1, 2, 3, 4};


    long long result[R_SIZE] = {0};
    
    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    mult(p1, p2, result);
    for(int i = 0;i<R_SIZE;i++){
        printf("%lld\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
