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
/*
int check0s(unsigned long long p[]){
    unsigned long long mask = 0x0000000000000000;
    printf("Value of number @ index 3: %d\n", p[3]);
    return ((p[3] & mask) == 0) ? 1 : 0;
}
*/
/*
int check0s(unsigned long long p[]){
    unsigned long long lsb = p[3] & 1ULL;
    printf("In check0s: p[3] = %llu, p[3] & 1ULL = %llu\n", p[3], lsb);
    return (lsb == 0ULL) ? 1 : 0;
}
*/

int check0s(unsigned long long p[]){
    unsigned long long mask = 1ULL;
    printf("p[3] = %llu - mask = %llu\n", p[3], mask);
    return ((p[3] & mask) == 0ULL) ? 1 : 0;
}


/*
int check1s(unsigned long long p[]){
    unsigned long long mask = 0xFFFFFFFFFFFFFFFF;
    return ((p[3] & mask) == 1) ? 1 : 0;
}
*/

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
    //unsigned long long p1[SIZE] = {0x3C134124B7F5C593, 0x58BC963551FADEF0, 0x4153F9BF96E563D2, 0x3BF29687E0FE8C77};
    //unsigned long long p2[SIZE] = {0xAC7242DBB8A9C3C1, 0xE994E8BEE7E830E4, 0xFC2D0E635929E38F, 0x5B5C6B76C6E0A497};

    //unsigned long long p1[SIZE] = {0, 0, 0, 4};
    //unsigned long long p2[P_SIZE] = {0xA7081AEA3BDBF56E, 0x0AE5736BE1124F8D, 0xC7CE6E75FAC521DD, 0x9F6A6B593208CDF6, 0x0E83615E354157D9};
    unsigned long long result[SIZE] = {0};


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
