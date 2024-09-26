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

void mult(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]){
    int i,j;
    __int128 temp = 0;
    for(i = 0;i<SIZE;i++){
        uint64_t carry = 0;
        for(j = 0;j<SIZE;j++){
            // (carry,sum) <- r[i+j] + a[j] * b[i] + carry
            temp = mult128(p1[j], p2[i]);
            temp = r[i+j] + temp + carry;
            r[i+j] = temp;
            carry = temp >> 64;
        }
        //any excess is added to the next element
        r[i+j+1] = carry;
    }
}
void sub_with_borrow(long long p1[], long long p2[], long long r[]){
    long long borrow = 0;
    for(int i = 0;i<SIZE;i++){
        //difference
        r[i] = p1[i] - p2[i] - borrow;
        borrow = (p1[i] < p2[i] + borrow);
    }
}

void reduc(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]){
    //SIZE + 1
    int b_k = 5;
    //b_b^(2*b_k) / p2[0]
    __uint128_t big_b = ((__uint128_t)b_b) << (2*b_k);
    unsigned long long mu = big_b / p2[0];
    printf("Value of mu: %llu\n", mu);
    unsigned long long b_mask = 1ULL << ((b_w * (b_k + 1))-1);
    printf("Value of b_mask: %llX\n", b_mask);
    unsigned long long b_expo = (b_b << (b_w * (b_k + 1)));
    printf("Value of b_expo: %llX\n", b_expo);
    __uint128_t qh = 0;
    //[7,6,5,4,3,2,1,0]
    //[0,0,0,0,0,0,7,6]
    for(int i =0;i<R_SIZE;i++){
        printf("Value of p1 @ i: %016llX - %d\n", p1[i], i);
    }
    unsigned long long temp[SIZE] = {0};
    for(int i = 0;i<SIZE;i++){
        temp[i] = p1[i+4];
        printf("Value of temp @ i: %llX - %d\n", temp[i], i);
    }
    int numShouldBe384Bits = (b_w * (b_k + 1));
    printf("Decimal value of bw * (b_k + 1) should be 384,  just making sure. Value = %d\n", numShouldBe384Bits);
    unsigned long long mu_array[SIZE] = {3, 5, 2, 9};
    unsigned long long result_to_be_shifted[R_SIZE] = {0};
    //qh = (temp * mu) >> numShouldBe384Bits;
    mult(temp, mu_array, result_to_be_shifted);
    //qh = result_to_be_shifted >> (b_w * (b_k + 1));
    __uint128_t lastElement = (__uint128_t)result_to_be_shifted[7];
    __uint128_t firstTwoElements = ((lastElement << 64) | result_to_be_shifted[6]);
    qh = ((lastElement << 64) | result_to_be_shifted[6]);

    printf("---qh---\n");
    printf("Value of qh: upper=%016llX, lower=%016llX\n", 
           (unsigned long long)(qh >> 64), 
           (unsigned long long)(qh & 0xFFFFFFFFFFFFFFFF));
    //rs = sub_with_borrow(p1, mult(qh, p2), r);
    //reduce p1 with mod and pass it to the subtraction function
    unsigned long long reduced_p1[SIZE] = {0};
    unsigned long long multRes[R_SIZE] = {0};
    unsigned long long temp_qh[SIZE] = {0};
    temp_qh[0] = (qh >> 64);
    temp_qh[1] = (qh & 0xFFFFFFFFFFFFFFFF);
    mult(temp_qh, p2, multRes);
    //then the pass the result of the mult of qh and p2[]
    sub_with_borrow(reduced_p1, multRes, r);
     
    

}

int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    unsigned long long p1[R_SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    unsigned long long p2[SIZE] = {0x2523648240000002, 0xe101d68000000016, 0x175700000000004d, 0x1900000000000067};


    unsigned long long result[R_SIZE] = {0};

    
    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    reduc(p1, p2, result);
    for(int i = R_SIZE;i>=0;i--){
        printf("%016llX\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
