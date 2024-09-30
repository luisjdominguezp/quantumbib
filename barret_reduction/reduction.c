#include <stdio.h>
#include <inttypes.h>
#include <x86intrin.h>
#include <gmp.h>

#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000
#define b_w 64

void measured_function(volatile int *var) {(*var) = 1; }

void reduc(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]){
    //SIZE + 1
    int b_k = 5;
    //gmp library was imported to handle big variables such as b_b, mu, b_mask, b_expo
    mpz_t b_b, p, big_b_pow, mu, b_mask, b_expo;
    mpz_inits(b_b, p, big_b_pow, mu, b_mask, b_expo, NULL);
    //b_b = 2^b_w
    mpz_ui_pow_ui(b_b, 2, b_w);
    //b_b^(2*b_k) / p2
    //p being modulus
    //imports p2[] into a number
    mpz_import(p, SIZE, 1, sizeof(unsigned long long), 0, 0, p2);
    gmp_printf("Value of imported p: %Zx\n", p);
    //b_b^2*b_k
    mpz_pow_ui(big_b_pow, b_b, 2*b_k);
    //mu = b_b^2*b_k / p
    mpz_fdiv_q(mu, big_b_pow, p);
    gmp_printf("Value of mu: %Zx\n", mu);
    //b_mask = 2^(b_w*(b_k+1))-1
    mpz_ui_pow_ui(b_mask, 2, b_w * (b_k + 1));
    mpz_sub_ui(b_mask, b_mask, 1);
    gmp_printf("Value of b_mask: %Zx\n", b_mask);
    //e^384
    mp_bitcnt_t exponent = 64 * (b_k + 1);
    gmp_printf("Value of exponent: %lu\n", exponent);
    //b_b^(b_k+1)
    mpz_pow_ui(b_expo, b_b, b_k + 1);
    gmp_printf("Value of b_expo: %Zx\n", b_expo);
    //__uint128_t qh = 0;
    //[7,6,5,4,3,2,1,0]
    //[0,0,0,0,0,0,7,6]
    /*
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
    unsigned long long mu_array[SIZE] = {};
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
    //reduced p1[] is mod(z, b_expo) and reduced multRes is mod(mult(qh, p2[]), b_expo)
    unsigned long long reduced_p1[SIZE] = {0};
    unsigned long long multRes[R_SIZE] = {0};
    unsigned long long temp_qh[SIZE] = {0};
    temp_qh[0] = (qh >> 64);
    temp_qh[1] = (qh & 0xFFFFFFFFFFFFFFFF);
    mult(temp_qh, p2, multRes);

    //then the pass the result of the mult of qh and p2[]
    sub_with_borrow(reduced_p1, multRes, r);
    */
    mpz_t z, qh, rs, temp, temp2, rsTemp, rsTemp2, rsT, mul2;
    mpz_inits(z, qh, rs, temp, temp2, rsTemp, rsTemp2, rsT, mul2, NULL);
    mpz_import(z, R_SIZE, 1, sizeof(unsigned long long), 0, 0, p1);
    //qh = (((z >> b_w*(b_k-1)) * mu) >> (b_w * (b_k + 1)))
    //temp = z >> b_w*(b_k-1) 
    mpz_fdiv_q_2exp(temp, z, b_w * (b_k - 1));
    //temp * mu
    mpz_mul(temp2, temp, mu);
    //temp2 >> b_w * (b_k + 1)
    mpz_fdiv_q_2exp(qh, temp2, b_w * (b_k + 1));
    gmp_printf("Value of qh: %Zx\n", qh);
    //mod(z, b_expo)
    mpz_mod_2exp(rsTemp, z, exponent);
    //mod(mul(qh, p), b_expo)
    mpz_mul(mul2, qh, p);
    mpz_mod_2exp(rsTemp2, mul2, exponent);
    mpz_sub(rsT, rsTemp, rsTemp2);

    if(mpz_sgn(rsT) < 0){
        mpz_add(rsT, rsT, b_expo);
    }

    while(mpz_cmp(rsT, p) >= 0) {
        mpz_sub(rsT, rsT, p);
    }
    //variable used to store number of blocks used in export
    size_t count;


    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, rsT);
    //printf("Number of blocks or limbs exported: %zu\n", count);

    mpz_clears(b_b, p, big_b_pow, mu, b_expo, z, qh, temp, temp2, rsT, rsTemp, rsTemp2, mul2, NULL);

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
    printf("Result is: ");
    for(int i = 0;i<SIZE;i++){
        printf("%016llX", result[i]);
    }
    end = __rdtsc();

    printf("\nTotal = %f CPU cycles", (float)(end - start) / NTEST);

    return 0;
}
