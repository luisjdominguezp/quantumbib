#include <bits/time.h>
#include <criterion/internal/test.h>
#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include <x86intrin.h>
#include <gmp.h>
#include "addition/addition.h"
#include "random/random.h"
#include "subtraction/subtracion.h"
#include "multiplication/multiplication.h"
#include "barrett_reduction/reduction.h"
#include "exponentiation/expo.h"
#include "montgomery/montgomery.h"
#include "mont_expo/mont_expo.h"
#include "sqrt_tonelli_shanks/sqrt.h"
#include "mod_inv/inv.h"
#include "check0s/check0s.h"
#include "check1s/check1s.h"
#include "hash/sha3.h"

// Copyright © 2024 Horacio Hernandez

#define SIZE 4
#define P_SIZE 5
#define R_SIZE 8
#pragma instrinic(__rdtsc)
#define NTEST 10000
#define BIT_LIMIT 64
#define bw 64
#define MOD_HEX "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF" 

int main(){
    uint64_t start, end;
    clock_t t;
    double time_taken = 0;
    int variable = 0;

    unsigned long long p1[SIZE] = {0};
    unsigned long long p2[SIZE] = {0};
    unsigned long long result[R_SIZE] = {0};
    unsigned long long p_expo[SIZE] = {0x1900000000000067, 0x175700000000004d, 0xe101d68000000016, 0x2523648240000002};
    unsigned long long mont_expo[SIZE] = {0xFFFFFFFF00000001, 0x0000000000000000, 0x00000000FFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    unsigned long long prime[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF,0x0000000000000000, 0xFFFFFFFF00000001};
    unsigned long long modInv[SIZE] = {0x3C134124B7F5C593, 0x58BC963551FADEF0, 0x4153F9BF96E563D2, 0x3BF29687E0FE8C77};
    unsigned long long prime2[P_SIZE] = {0xA7081AEA3BDBF56E, 0x0AE5736BE1124F8D, 0xC7CE6E75FAC521DD, 0x9F6A6B593208CDF6, 0x0E83615E354157D9};
    mpz_t mod;
    mpz_init(mod);

    mpz_set_str(mod, MOD_HEX, 16);
   
    int choice = 0;
    int option = 0;
    int option2 = 0;

    while(1){
        printf("----Enter 1 to input your numbers manually, 2 for random numbers or 3 to exit----\n");
        if(scanf("%d", &choice) != 1){
            printf("Invalid input. Exiting.\n");
            break;
        }

        if(choice == 1){
            for(int i = 0; i < SIZE; i++){
                printf("Enter your %dth number in limbs of 64 bits: ", i+1);
                if(scanf("%llX", &p1[i]) != 1){
                    printf("Invalid input. Exiting.\n");
                    return 1;
                }
            }
            for(int i = 0; i < SIZE; i++){
                printf("Enter your %dth number in limbs of 64 bits: ", i+1);
                if(scanf("%llX", &p2[i]) != 1){
                    printf("Invalid input. Exiting.\n");
                    return 1;
                }
            }

        } else if(choice == 2) {        
            gmp_randstate_t state;
            gmp_randinit_default(state);

            unsigned long seed = (unsigned long)time(NULL);
            gmp_randseed_ui(state, seed);
            q_random(p1, state, SIZE);
            q_random(p2, state, SIZE);
            gmp_randclear(state);
        }  else if (choice == 3) {
            printf("Exiting program...\n");
            break;
        } else {
            printf("Invalid choice. Please enter 1, 2 or 3.\n");
            continue;
        }

        for(int i = 0; i < SIZE; i++){
            printf("Content of p1: %016llX\n", p1[i]);
        }
        for(int i = 0; i < SIZE; i++){
            printf("Content of p2: %016llX\n", p2[i]);
        }

        printf("----Select Operation----\n");
        printf("1) Addition\n");
        printf("2) Subtraction\n");
        printf("3) Multiplication\n");
        printf("4) Barrett Reduction\n");
        printf("5) Exponentiation\n");
        printf("6) Montgomery Product\n");
        printf("7) Montgomery Exponentiation\n");
        printf("8) Square root\n");
        printf("9) Modular inverse\n");
        printf("10) Check if LSB is 0\n");
        printf("11) Check is LSB is 1\n");
        printf("12) Hash\n");
        printf("13) Exit\n");
        printf("Enter your choice: ");

        if(scanf("%d", &choice) != 1){
            printf("Invalid input. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1:
                printf("Starting addition of these 2 numbers...\n");
                printf("Calculating CPU cycles...\n");
                start = __rdtsc();
                t = clock();
                for(int i =0;i<NTEST;i++){
                    add_with_carry(p1, p2, result, SIZE);
                }
                t = clock() - t;
                for(int i=0;i<SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                time_taken = ((double)t/CLOCKS_PER_SEC);
                printf("Execution time: %f seconds\n", time_taken);
                break;
            case 2:
                printf("Starting subtraction of these 2 numbers...\n");
                printf("Calculating CPU cycles...\n");
                start = __rdtsc();
                t = clock();
                for(int i =0;i<NTEST;i++){
                    sub_with_borrow(p1, p2, result, SIZE);
                }
                t = clock() - t;
                end = __rdtsc();
                for(int i=0;i<SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                time_taken = ((double)t/CLOCKS_PER_SEC);
                printf("Execution time: %f seconds\n", time_taken);
                break;
            case 3:
                printf("Starting multiplication of these 2 numbers...\n");
                printf("Calculating CPU cycles...\n");
                start = __rdtsc();
                t = clock();
                for(int i =0;i<NTEST;i++){
                    mult(p1, p2, result, SIZE, R_SIZE);
                }
                t = clock() - t;
                end = __rdtsc();
                for(int i=0;i<R_SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                time_taken = ((double)t/CLOCKS_PER_SEC);
                printf("Execution time: %f seconds\n", time_taken);
                break;
            case 4:
                printf("Starting Barrett reduction of these 2 numbers...\n");
                printf("Calculating CPU cycles...\n");
                start = __rdtsc();
                t = clock();
                for(int i =0;i<NTEST;i++){
                    reduc(p1, p_expo, result, SIZE, R_SIZE, bw);
                }
                t = clock() - t;
                end = __rdtsc();
                for(int i=0;i<SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                time_taken = ((double)t/CLOCKS_PER_SEC);
                printf("Execution time: %f seconds\n", time_taken);
                break;
            case 5:
                printf("Starting exponentiation of these 2 numbers...\n");
                printf("Calculating CPU cycles...\n");
                start = __rdtsc();
                t = clock();
                for(int i =0;i<NTEST;i++){
                    expo(p1, prime, result, SIZE, BIT_LIMIT);
                }
                t = clock() - t;
                end = __rdtsc();
                for(int i=0;i<SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                time_taken = ((double)t/CLOCKS_PER_SEC);
                printf("Execution time: %f seconds\n", time_taken);
                break;
            case 6:
                printf("Starting montgomery product of these 2 numbers...\n");
                printf("Calculating CPU cycles...\n");
                start = __rdtsc();
                t = clock();
                for(int i =0;i<NTEST;i++){
                    montgomery_pr(p1, p2, result, SIZE, BIT_LIMIT, mod);
                }
                t = clock() - t;
                end = __rdtsc();
                for(int i=0;i<R_SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                time_taken = ((double)t/CLOCKS_PER_SEC);
                printf("Execution time: %f seconds\n", time_taken);
                break;
            case 7:
                printf("Starting montgomery exponentiation of these 2 numbers...\n");
                printf("Calculating CPU cycles...\n");
                start = __rdtsc();
                t = clock();
                for(int i =0;i<NTEST;i++){
                    montgomery_exp(p1, mont_expo, result, SIZE, BIT_LIMIT, mod);
                }
                t = clock() - t;
                end = __rdtsc();
                for(int i=0;i<R_SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                time_taken = ((double)t/CLOCKS_PER_SEC);
                printf("Execution time: %f seconds\n", time_taken);
                break;
            case 8:
                printf("Starting square root operation...\n");
                printf("Calculating CPU cycles...\n");
                start = __rdtsc();
                t = clock();
                for(int i =0;i<NTEST;i++){
                    t_sqrt(p1, prime2, result, SIZE, R_SIZE);
                }
                t = clock() - t;
                end = __rdtsc();
                if(result[0] == (unsigned long long)-1){
                    printf("No square root exists.\n");
                } else {
                    for(int i=R_SIZE-1;i>=0;i--){
                        printf("Resulting Array: %016llX\n", result[i]);
                    }
                }
                printf("Verifying computed square root...\n");
                int verification_result = verify_sqrt(p1, prime2, result, SIZE, P_SIZE, R_SIZE);
                if (verification_result) {
                    printf("Verification passed: Computed square root is correct.\n");
                } else {
                    printf("Verification failed: Computed square root is incorrect.\n");
                }
                printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                time_taken = ((double)t/CLOCKS_PER_SEC);
                printf("Execution time: %f seconds\n", time_taken);
                break;
            case 9:
                printf("Starting modular inverse...\n");
                printf("Calculating CPU cycles...\n");
                int status = 0;
                start = __rdtsc();
                t = clock();
                for(int i=0;i<NTEST;i++){
                    status = inv_mod(modInv, prime2, result, SIZE);
                }
                t = clock() - t;
                end = __rdtsc();
                if (status != 1) {
                    printf("Modular inverse does not exist for these values.\n");
                } else {
                    printf("Modular inverse is:\n");
                    for(int i=0;i<SIZE;i++){
                        printf("%016llX\n", result[i]);
                    }
                }
                printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                time_taken = ((double)t/CLOCKS_PER_SEC);
                printf("Execution time: %f seconds\n", time_taken);
                break;
            case 10:
                printf("Choose which number to check: (0 - p1) (1 - p2)\n");
                if(scanf("%d", &option) !=1){
                    printf("Invalid input. Exiting.\n");
                    break;
                }
                if(option==0){
                    int res1 = 0;
                    printf("Starting check0s on this number...\n");
                    printf("Calculating CPU cycles...\n");
                    start = __rdtsc();
                    t = clock();
                    for(int i=0;i<NTEST;i++){
                        int resP1 = check0s(p1, SIZE);
                        res1 = resP1;
                    }
                    t = clock() - t;
                    end = __rdtsc();
                    printf("Result for this number is: %d (0-False 1-True)\n", res1);
                    printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                    time_taken = ((double)t/CLOCKS_PER_SEC);
                    printf("Execution time: %f seconds\n", time_taken);
                } else if (option==1) {
                    int res2 = 0;
                    printf("Starting check0s on this number...\n");
                    printf("Calculating CPU cycles...\n");
                    start = __rdtsc();
                    t = clock();
                    for(int i = 0;i<NTEST;i++){
                        int resP2 = check0s(p2, SIZE);
                        res2 = resP2;
                    }
                    t = clock() - t;
                    end = __rdtsc();
                    printf("Result for this number is: %d (0-False 1-True)\n", res2);
                    printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                    time_taken = ((double)t/CLOCKS_PER_SEC);
                    printf("Execution time: %f seconds\n", time_taken);
                } else {
                    printf("Invalid option.\n");
                }
                break;
            case 11:
                printf("Choose which number to check: (0 - p1) (1 - p2)\n");
                if(scanf("%d", &option2) !=1){
                    printf("Invalid input. Exiting.\n");
                    break;
                }
                if(option==0){
                    int res1 = 0;
                    printf("Starting check1s on this number...\n");
                    printf("Calculating CPU cycles...\n");
                    start = __rdtsc();
                    t = clock();
                    for(int i=0;i<NTEST;i++){
                        int resP1 = check1s(p1, SIZE);
                        res1 = resP1;
                    }
                    t = clock() - t;
                    end = __rdtsc();
                    printf("Result for this number is: %d (0-False 1-True)\n", res1);
                    printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                    time_taken = ((double)t/CLOCKS_PER_SEC);
                    printf("Execution time: %f seconds\n", time_taken);
                } else if (option==1) {
                    int res2 = 0;
                    printf("Starting check1s on this number...\n");
                    printf("Calculating CPU cycles...\n");
                    start = __rdtsc();
                    t = clock();
                    for(int i = 0;i<NTEST;i++){
                        int resP2 = check1s(p2, SIZE);
                        res2 = resP2;
                    }
                    t = clock() - t;
                    end = __rdtsc();
                    printf("Result for this number is: %d (0-False 1-True)\n", res2);
                    printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                    time_taken = ((double)t/CLOCKS_PER_SEC);
                    printf("Execution time: %f seconds\n", time_taken);
                } else {
                    printf("Invalid option\n");
                }
                break;
            case 12:
                printf("Choose which number to hash: (0 - p1) (1 - p2)\n");
                if(scanf("%d", &option2) !=1){
                    printf("Invalid input. Exiting.\n");
                    break;
                }
                if(option==0){
                    printf("Starting hash of data...\n");
                    unsigned char digest[32];
                    printf("Calculating CPU cycles...\n");
                    start = __rdtsc();
                    t = clock();
                    for(int i =0;i<NTEST;i++){
                        hash_sha3_256((unsigned char *)p1, SIZE, digest);
                    }
                    t = clock() - t;
                    end = __rdtsc();
                    printf("SHA3-256 Digest: ");
                    for(int i =0;i<32;i++){
                        printf("%02X", digest[i]);
                    }
                    printf("\n");
                    printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                    time_taken = ((double)t/CLOCKS_PER_SEC);
                    printf("Execution time: %f seconds\n", time_taken);
                } else if (option==1){
                    printf("Starting hash of data...\n");
                    unsigned char digest[32];
                    printf("Calculating CPU cycles...\n");
                    start = __rdtsc();
                    t = clock();
                    for(int i =0;i<NTEST;i++){
                        hash_sha3_256((unsigned char *)p2, SIZE, digest);
                    }
                    t = clock() - t;
                    end = __rdtsc();
                    printf("SHA3-256 Digest: ");
                    for(int i =0;i<32;i++){
                        printf("%02X", digest[i]);
                    }
                    printf("\n");
                    printf("Total = %f CPU cycles\n", (float)(end-start)/NTEST);
                    time_taken = ((double)t/CLOCKS_PER_SEC);
                    printf("Execution time: %f seconds\n", time_taken);
                } else {
                    printf("Invalid input. Exiting\n");
                }
                break;
            case 13:
                printf("Exiting program.\n");
                return 0;
            default:
                printf("Invalid operation choice.\n");
                break;
        }
    }

    mpz_clear(mod);
    return 0;

    
}
