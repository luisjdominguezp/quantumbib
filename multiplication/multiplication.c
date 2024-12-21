#include <stdio.h>
#include <inttypes.h>
#include <x86intrin.h>
/*
#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000


void measured_function(volatile int *var) {(*var) = 1; }
*/
// Copyright © 2024 Horacio Hernandez
__int128 mult128(__int128 p1, __int128 p2){
    return p1*p2;
}


void mult(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int size, int r_size){
    int i,j = 0;
    for(int i=0;i<size;i++){
        r[i] = 0;
    }
    __int128 temp = 0;
    for(i = 0;i<r_size;i++){
        uint64_t carry = 0;
        for(j = 0;j<size;j++){
            // (Carry,Sum) <- r[i+j] + a[j] * b[i] + Carry
            temp = mult128(p1[j], p2[i]);
            temp = r[i+j] + temp + carry;
            r[i+j] = temp;
            carry = temp >> 64;
        }
        //any excess is added to the next element
        if((i+j) < r_size){
            r[i+j] += carry;
        }
    }
}
/*
int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p1[SIZE] = {2, 4, 6, 8};
    //unsigned long long p2[SIZE] = {1, 2, 3, 4};


    unsigned long long result[R_SIZE] = {0};

    
    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    for(int i =0;i<NTEST;i++){
        mult(p1, p2, result);
    }

    mult(p1, p2, result);
    for(int i = 0;i<R_SIZE;i++){
        printf("%016llX\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
*/
