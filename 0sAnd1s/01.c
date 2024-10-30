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

int check0s(unsigned long long p1[], unsigned long long p2[]){

}

int check1s(unsigned long long p1[], unsigned long long p2[]){

}

int main(){
    uint64_t start, end;
    int variable = 0;

    unsigned long long p1[SIZE] = {0x3C134124B7F5C593, 0x58BC963551FADEF0, 0x4153F9BF96E563D2, 0x3BF29687E0FE8C77};
    unsigned long long p2[SIZE] = {0xAC7242DBB8A9C3C1, 0xE994E8BEE7E830E4, 0xFC2D0E635929E38F, 0x5B5C6B76C6E0A497};

    //unsigned long long p1[SIZE] = {0, 0, 0, 4};
    //unsigned long long p2[P_SIZE] = {0xA7081AEA3BDBF56E, 0x0AE5736BE1124F8D, 0xC7CE6E75FAC521DD, 0x9F6A6B593208CDF6, 0x0E83615E354157D9};
    unsigned long long result[SIZE] = {0};


    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    check0s(p1, p2);
    check1s(p1, p2);
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;

}
