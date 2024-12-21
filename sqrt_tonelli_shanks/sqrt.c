#include <signal.h>
#include <stdio.h>
#include <gmp.h>
#include <inttypes.h>
#include <x86intrin.h>
/*
#define SIZE 4
#define P_SIZE 5
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000
#define BIT_LIMIT 64

void measured_function(volatile int *var) {(*var) = 1; }
*/
// Copyright © 2024 Horacio Hernandez
int verify_sqrt(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int size, int p_size, int r_size) {
    mpz_t n_mpz, r_mpz, p_mpz, temp;
    mpz_inits(n_mpz, r_mpz, p_mpz, temp, NULL);
    
    mpz_import(n_mpz, size, 1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(p_mpz, p_size, 1, sizeof(unsigned long long), 0, 0, p2);
    mpz_import(r_mpz, r_size, -1, sizeof(unsigned long long), 0, 0, r);

    //gmp_printf("Value of n_mpz: %Zd\n", n_mpz);
    //gmp_printf("Value of p_mpz: %ZX\n", p_mpz);
    //gmp_printf("Value of r_mpz: %ZX\n", r_mpz);
    
    mpz_powm_ui(temp, r_mpz, 2, p_mpz);
    
    int result = (mpz_cmp(temp, n_mpz) == 0);
    
    if (result) {
        printf("Square root verification PASSED\n");
        printf("r^2 mod p equals n mod p\n");
    } else {
        printf("Square root verification FAILED\n");
        //gmp_printf("r^2 mod p = %Zx\n", r_mpz);
        //gmp_printf("n mod p = %Zx\n", temp);
    }
    
    mpz_clears(n_mpz, r_mpz, p_mpz, temp, NULL);
    return result;
}

void t_sqrt(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int size, int p_size) {
        mpz_t p, n, r_mpz, q, s, z, c, t, m, b, temp, exponent;
    unsigned long e;
    int found = 0;

    mpz_inits(p, n, r_mpz, q, s, z, c, t, m, b, temp, exponent, NULL);

    mpz_import(n, size, 1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(p, p_size, 1, sizeof(unsigned long long), 0, 0, p2);

    //gmp_printf("Value of p1: %Zd\n", n);
    //gmp_printf("Value of p2: %ZX\n", p);
    if (mpz_legendre(n, p) != 1) {
        //gmp_printf("No square root exists.\n");
        mpz_clears(p, n, r_mpz, q, s, z, c, t, m, b, temp, exponent, NULL);
        return;
    }

    mpz_sub_ui(q, p, 1);  // q = p - 1
    mpz_set_ui(s, 0);
    while (mpz_even_p(q)) {
        mpz_fdiv_q_2exp(q, q, 1);  // q = q / 2
        mpz_add_ui(s, s, 1);       // s = s + 1
    }

    mpz_set_ui(z, 2);
    while (!found) {
        if (mpz_legendre(z, p) == -1) {
            found = 1;
        } else {
            mpz_add_ui(z, z, 1);
        }
    }

    mpz_powm(c, z, q, p);

    mpz_add_ui(temp, q, 1);                   // temp = Q + 1
    mpz_fdiv_q_2exp(exponent, temp, 1);       // exponent = (Q + 1) / 2

    mpz_powm(r_mpz, n, exponent, p);

    mpz_powm(t, n, q, p);

    mpz_set(m, s);

    while (mpz_cmp_ui(t, 1) != 0) {
        mpz_set(temp, t);
        e = 0;
        while (mpz_cmp_ui(temp, 1) != 0 && e < mpz_get_ui(m)) {
            mpz_powm_ui(temp, temp, 2, p);
            e++;
        }
        if (e >= mpz_get_ui(m)) {
            //gmp_printf("No square root exists.\n");
            mpz_clears(p, n, r_mpz, q, s, z, c, t, m, b, temp, exponent, NULL);
            return;
        }

        mpz_sub_ui(temp, m, e + 1);                  
        mpz_ui_pow_ui(temp, 2, mpz_get_ui(temp));
        mpz_powm(b, c, temp, p);

        mpz_mul(r_mpz, r_mpz, b);
        mpz_mod(r_mpz, r_mpz, p);

        mpz_powm_ui(b, b, 2, p);  // b = b^2 mod p
        mpz_mul(t, t, b);
        mpz_mod(t, t, p);

        mpz_set(c, b);

        mpz_set_ui(m, e);
    }

    size_t count;
    mpz_export(r, &count, -1, sizeof(unsigned long long), 0, 0, r_mpz);
    mpz_clears(p, n, r_mpz, q, s, z, c, t, m, b, temp, exponent, NULL);

}

/*
int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    unsigned long long p1[SIZE] = {0, 0, 0, 4};
    unsigned long long p2[P_SIZE] = {0xA7081AEA3BDBF56E, 0x0AE5736BE1124F8D, 0xC7CE6E75FAC521DD, 0x9F6A6B593208CDF6, 0x0E83615E354157D9};
    unsigned long long result[R_SIZE] = {0};


    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    for(int i=0;i<NTEST;i++){
        t_sqrt(p1, p2, result);
    }
    if (result[0] == (unsigned long long)-1){
        printf("No square root exists.\n");
    } else {
        for(int i = R_SIZE -1;i>=0;i--){
        printf("%016llX\n", result[i]);
        }
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    // Call the verification function
    printf("Verifying computed square root...\n");
    int verification_result = verify_sqrt(p1, p2, result);
    if (verification_result) {
        printf("Verification passed: Computed square root is correct.\n");
    } else {
        printf("Verification failed: Computed square root is incorrect.\n");
    }
    return 0;

}
*/
