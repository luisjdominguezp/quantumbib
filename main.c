#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include <x86intrin.h>
#include <gmp.h>
#include "addition/addition.h"
#include "random/random.h"
#include "subtraction/subtracion.h"
#include "multiplication/multiplication.h"
#include "barret_reduction/reduction.h"





#define SIZE 4
#define R_SIZE 8
#pragma instrinic(__rdstc)
#define NTEST 10000
#define bw 64


void measured_function(volatile int *var){(*var) = 1;}

int main(){
    uint64_t start, end;
    int variable = 0;

    unsigned long long p1[SIZE] = {0};
    unsigned long long p2[SIZE] = {0};
    unsigned long long result[SIZE] = {0};
   
    int choice = 0;

    while(1){
        while(1){
        printf("----Enter 1 to input your numbers manually or 2 for random numbers----\n");
        if(scanf("%d", &choice) != 1){
            printf("Invalid input. Exiting.\n");
            break;
        }

        if(choice == 1){
            for(int i = 0; i < SIZE; i++){
                printf("Enter your number in limbs of 64 bits: ", i+1);
                if(scanf("%llX", &p1[i]) != 1){
                    printf("Invalid input. Exiting.\n");
                    return 1;
                }
            }
            for(int i = 0; i < SIZE; i++){
                printf("Enter your number in limbs of 64 bits: ", i+1);
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
        } else {
            printf("Invalid choice. Please enter 1 or 2.\n");
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
        printf("5) Exit\n");
        printf("Enter your choice: ");

        if(scanf("%d", &choice) != 1){
            printf("Invalid input. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1:
                printf("Starting addition of these 2 numbers...\n");
                add_with_carry(p1, p2, result, SIZE);
                for(int i=0;i<SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                break;
            case 2:
                printf("Starting subtraction of these 2 numbers...\n");
                sub_with_borrow(p1, p2, result, SIZE);
                for(int i=0;i<SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                break;
            case 3:
                printf("Starting multiplication of these 2 numbers...\n");
                mult(p1, p2, result, SIZE, R_SIZE);
                for(int i=0;i<SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                break;
            case 4:
                printf("Starting Barrett reduction of these 2 numbers...\n");
                reduc(p1, p2, result, SIZE, R_SIZE);
                for(int i=0;i<SIZE;i++){
                    printf("Resulting Array: %016llX\n", result[i]);
                }
                break;
            case 5:
                printf("Exiting program.\n");
                return 0;
            default:
                printf("Invalid operation choice.\n");
                break;
        }
    }
    }

    printf("Warming up the cpu.\n");
    for(int i=0;i<NTEST;i++)
    {
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();

    end = __rdtsc();

    return 0;

    
}
