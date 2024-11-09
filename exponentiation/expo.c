#include <stdio.h>
#include <gmp.h>
#include <inttypes.h>
#include <x86intrin.h>
/*
#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000
#define BIT_LIMIT 64

void measured_function(volatile int *var) {(*var) = 1; }
*/
void expo(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int size, int r_size, int bit_limit){
    mpz_t base, expo, res;
    size_t count;
    mpz_inits(base, expo, res, NULL);
    mpz_import(base, size, -1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(expo, size, -1, sizeof(unsigned long long), 0, 0, p2);
    //gmp_printf("Value of base: %ZX\n", base);
    //gmp_printf("Value of expo: %ZX\n", expo);
    //check to see if expo is 0, 1 or 2
    /*
    if(mpz_probab_prime_p(expo, 25)) {
        gmp_printf("Exponent p is probably prime.\n");
    } else {
        gmp_printf("Exponent p is not prime.\n");
    }
    */

    size_t bits = mpz_sizeinbase(expo, 2);
    if(bits <= bit_limit){
        //check to see if expo is 0, 1 or 2
        if(mpz_cmp_ui(expo, 0) == 0){
            mpz_set_ui(res, 1);
            mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, res);
            mpz_clears(base, expo, res, NULL);
            return;
        }
        if(mpz_cmp_ui(expo, 1) == 0){
            mpz_set(res, base);
            mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, res);
            mpz_clears(base, expo, res, NULL);
            return;
        }
        if(mpz_cmp_ui(expo, 2) == 0){
            mpz_mul(res, base, base);
            mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, res);
            mpz_clears(base, expo, res, NULL);
            return;
        } else {
            if(bits == 0){
            bits = 1;
        }
            //g = f
            mpz_set(res, base);
                for(ssize_t i=bits-2;i>=0;i--){
                //printf("Processing bit %zd\n", i);
                //square
                mpz_mul(res, res, res);
                if(mpz_tstbit(expo, i)){
                    mpz_mul(res, res, base);
                }
            }
        }
    }
    else {
        //2^p mod p
        mpz_powm(res, base, expo, expo);
    }

    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, res);
    //printf("Number of blocks exported: %ld\n", count);
    mpz_clears(base, expo, res, NULL);
    

}
/*
int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    unsigned long long p1[SIZE] = {2, 0, 0, 0};
    unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF,0x0000000000000000, 0xFFFFFFFF00000001};
    //unsigned long long p2[SIZE] = {0x1900000000000067, 0x175700000000004d, 0xe101d68000000016, 0x2523648240000002};
    
    //unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0, 0, 0, 0};
    //unsigned long long p1[SIZE] = {2, 0, 0, 0};
    //unsigned long long p2[SIZE] = {5, 0, 0, 0};

    unsigned long long result[SIZE] = {0};


    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    for(int i =0;i<NTEST;i++){
        expo(p1, p2, result);
    }
    //expo(p1, p2, result);
    for(int i = 0;i<SIZE;i++){
        printf("%016llX\n", result[i]);
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
*/
