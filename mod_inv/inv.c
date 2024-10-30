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

int inv_mod(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]) {
    mpz_t a, n, u, v, x1, x2, temp, result_mpz;
    mpz_inits(a, n, u, v, x1, x2, temp, result_mpz, NULL);

    mpz_import(a, SIZE, 1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(n, SIZE, 1, sizeof(unsigned long long), 0, 0, p2);

    gmp_printf("Value of p1: %ZX\n", a);
    gmp_printf("Value of p2: %ZX\n", n);

    mpz_set(u, a);
    mpz_set(v, n);

    mpz_set_ui(x1, 1);
    mpz_set_ui(x2, 0);

    if (mpz_sgn(u) < 0) mpz_add(u, u, n);
    if (mpz_sgn(v) < 0) mpz_add(v, v, n);

    while (mpz_cmp_ui(u, 1) != 0 && mpz_cmp_ui(v, 1) != 0) {
        // Make u odd
        while (mpz_even_p(u)) {
            mpz_fdiv_q_2exp(u, u, 1); // u = u / 2

            if (mpz_even_p(x1)) {
                mpz_fdiv_q_2exp(x1, x1, 1); // x1 = x1 / 2
            } else {
                mpz_add(x1, x1, n);
                mpz_fdiv_q_2exp(x1, x1, 1); // x1 = (x1 + n) / 2
            }
        }

        while (mpz_even_p(v)) {
            mpz_fdiv_q_2exp(v, v, 1); // v = v / 2

            if (mpz_even_p(x2)) {
                mpz_fdiv_q_2exp(x2, x2, 1); // x2 = x2 / 2
            } else {
                mpz_add(x2, x2, n);
                mpz_fdiv_q_2exp(x2, x2, 1); // x2 = (x2 + n) / 2
            }
        }

        if (mpz_cmp(u, v) >= 0) {
            mpz_sub(u, u, v);         // u = u - v
            mpz_sub(x1, x1, x2);      // x1 = x1 - x2

            if (mpz_sgn(x1) < 0) {
                mpz_add(x1, x1, n);   // Ensure x1 >= 0
            }
        } else {
            mpz_sub(v, v, u);         // v = v - u
            mpz_sub(x2, x2, x1);      // x2 = x2 - x1

            if (mpz_sgn(x2) < 0) {
                mpz_add(x2, x2, n);   // Ensure x2 >= 0
            }
        }
    }

    if (mpz_cmp_ui(u, 1) == 0) {
        mpz_mod(result_mpz, x1, n); // result = x1 mod n
    } else {
        mpz_mod(result_mpz, x2, n); // result = x2 mod n
    }

    size_t count;
    mpz_export(r, &count, -1, sizeof(unsigned long long), 0, 0, result_mpz);

    mpz_clears(a, n, u, v, x1, x2, temp, result_mpz, NULL);
    return 1;
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
    int status = inv_mod(p1, p2, result);
    if (status != 1) {
        printf("Modular inverse does not exist for these values.\n");
    } else {
        printf("Modular inverse is:\n");
        for(int i=0;i<SIZE;i++){
        printf("%016llX\n", result[i]);
        }
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;

}
