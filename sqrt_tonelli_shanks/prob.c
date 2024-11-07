#include <stdio.h>
#include <gmp.h>
#include <time.h>

#define P_SIZE 5  // 5 limbs of 64 bits each, total 320 bits

int main() {
    mpz_t p;
    gmp_randstate_t state;
    unsigned long long p_array[P_SIZE] = {0};

    mpz_init(p);
    gmp_randinit_default(state);

    // Seed the random number generator
    unsigned long seed = (unsigned long)time(NULL);
    gmp_randseed_ui(state, seed);

    // Generate a random number with 320 bits
    mpz_urandomb(p, state, 320);

    // Ensure the number is odd (since even numbers > 2 are not prime)
    mpz_setbit(p, 0);

    // Find the next prime number greater than p
    mpz_nextprime(p, p);

    // Verify that p is prime
    int reps = 25;  // Number of Miller-Rabin repetitions
    int is_prime = mpz_probab_prime_p(p, reps);

    if (is_prime == 2) {
        printf("The number p is definitely prime.\n");
    } else if (is_prime == 1) {
        printf("The number p is probably prime.\n");
    } else {
        printf("The number p is composite.\n");
        return 1;  // Exit if p is not prime
    }

    // Output the prime number
    printf("Generated prime p:\n");
    mpz_out_str(stdout, 16, p);
    printf("\n");

    // Export the prime number to an array
    size_t count;
    mpz_export(p_array, &count, -1, sizeof(unsigned long long), 0, 0, p);

    // Zero any remaining elements if mpz_export didn't fill the array
    for (size_t i = count; i < P_SIZE; i++) {
        p_array[i] = 0;
    }

    // Print the array elements
    printf("p_array (least significant limb first):\n");
    for (int i = 0; i < P_SIZE; i++) {
        printf("0x%016llX,\n", p_array[i]);
    }

    mpz_clear(p);
    gmp_randclear(state);

    return 0;
}

