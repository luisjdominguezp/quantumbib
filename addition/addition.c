#include <stdio.h>
#include <inttypes.h>
#include <x86intrin.h>
#define SIZE 4

#pragma intrinsic(__rdtsc)

#define NTEST 100000


void measured_function(volatile int *var) {(*var) = 1; }

void add_with_carry(long long p1[], long long p2[], long long r[]){
    long long carry = 0;
    for(int i = 0; i<SIZE;i++){
        //I'm carrying any leftover overflow to the second number 
        //and then to the first number
        unsigned long long tmpSum = p2[i] + carry;
        r[i] = p1[i] + tmpSum;
        //if both are true carry = 1
        //basically meaning there was an overflow
        carry = (p1[i] > r[i]) | (p2[i] > tmpSum);
    }

} 

int main(){
    uint64_t start, end;
    int variable = 0;

    unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    unsigned long long result[SIZE] = {0};
    
    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    add_with_carry(p1, p2, result);
    for(int i = 0;i<SIZE;i++){
        printf("%X\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);


    /*
    printf("Result: ");
    for(int i = 0;i<SIZE;i++){
        printf("%X ", result[i]);
    }
    printf("End of result.\n");
    */
    return 0;
}
