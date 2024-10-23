#include <math.h>
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

int legendre(mpz_t a, mpz_t p){
    int k = 1;
    mpz_t A, B, temp;
    mpz_init_set(A, a);
    mpz_init_set(B, p);
    mpz_init(temp);

    while (mpz_cmp_ui(B, 1) != 0) {
        if (mpz_cmp_ui(A, 0) == 0) {
            mpz_clears(A, B, temp, NULL);
            return 0;
        }

        int v = 0;
        while (mpz_even_p(A)) {
            v += 1;
            mpz_fdiv_q_2exp(A, A, 1); // A = A / 2
        }

        // t0 = B mod 8
        unsigned long t0 = mpz_fdiv_ui(B, 8);

        if ((v % 2 == 1) && (t0 == 3 || t0 == 5)) {
            k = -k;
        }

        // A_mod_4 = A mod 4
        unsigned long A_mod_4 = mpz_fdiv_ui(A, 4);
        // B_mod_4 = B mod 4
        unsigned long B_mod_4 = mpz_fdiv_ui(B, 4);

        if ((A_mod_4 == 3) && (B_mod_4 == 3)) {
            k = -k;
        }

        mpz_t r;
        mpz_init_set(r, A);
        mpz_mod(A, B, r);
        mpz_set(B, r);

        mpz_clear(r);
    }

    if (mpz_cmp_ui(B, 1) > 0) {
        k = 0;
    }

    mpz_clears(A, B, temp, NULL);

    return k;
}

void t_sqrt(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]) {
    mpz_t N, p, f, result, x_expected, p_minus_r, exp;
    mpz_inits(N, p, f, result, x_expected, p_minus_r, exp, NULL);

    mpz_import(N, R_SIZE, 1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(p, SIZE, 1, sizeof(unsigned long long), 0, 0, p2);

    mpz_mod(f, N, p);

    gmp_printf("Value of N: %ZX\n", N);
    gmp_printf("Value of modulus p: %ZX\n", p);
    gmp_printf("Value of f (N mod p): %ZX\n", f);

    // Check if f is a quadratic residue modulo p
    if (mpz_legendre(f, p) != 1) {
        // No square root exists
        mpz_set_ui(result, 0);
        printf("No square root exists.\n");
    } else {
        // For p ≡ 3 mod 4, we can compute sqrt(f) as f^((p+1)/4) mod p
        mpz_add_ui(exp, p, 1);  // exp = p + 1
        mpz_fdiv_q_2exp(exp, exp, 2);  // exp = (p + 1) / 4
        
        gmp_printf("Exp: %ZX\n", exp);
        // Compute result = f^{(p + 1) / 4} mod p
        mpz_powm(result, f, exp, p);
        gmp_printf("Result: %ZX\n", result);
        // Verify that (result^2 mod p) == f
        mpz_powm_ui(p_minus_r, result, 2, p);
        if (mpz_cmp(p_minus_r, f) == 0) {
            printf("Verification succeeded: (result)^2 mod p == f\n");
        } else {
            printf("Verification failed: (result)^2 mod p != f\n");
            mpz_set_ui(result, 0);
        }

        mpz_sub(p_minus_r, p, result);
        gmp_printf("Result: %ZX\n", result);
        gmp_printf("p-r: %ZX\n", p_minus_r);

        if (mpz_cmp(result, x_expected) == 0) {
            printf("Computed square root equals the expected x.\n");
        } else if (mpz_cmp(p_minus_r, x_expected) == 0) {
            printf("p - result equals the expected x.\n");
        } else {
            printf("Neither result nor p - result equals the expected x.\n");
        }
    }

    size_t count;
    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, result);

    mpz_clears(N, p, f, result, x_expected, p_minus_r, exp, NULL);
}


int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    //unsigned long long p1[SIZE] = {0, 0, 0, 9};
    //FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF
    unsigned long long p1[R_SIZE] = {0xFFFFFFFF00000003, 0xFFFFFFFD00000004, 0x0000000200000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000};
    //unsigned long long p2[SIZE] = {0xFFFFFFFF00000001, 0x0000000000000000, 0x00000000FFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    unsigned long long p2[R_SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE, 0xBAAEDCE6AF48A03B, 0xBFD25E8CD0364141};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF, 0x0000000000000000, 0xFFFFFFFF00000001};
    //unsigned long long p2[SIZE] = {0x1900000000000067, 0x175700000000004d, 0xe101d68000000016, 0x2523648240000002};
    
    //unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p1[SIZE] = {0x00000000FFFFFFFE, 0xFFFFFFFF00000001, 0x0000000000000000, 0x0000000000000000};
    //unsigned long long p2[SIZE] = {0, 0, 0, 0};
    //unsigned long long p2[SIZE] = {2, 0, 0, 0};
    //unsigned long long p2[SIZE] = {0, 0, 0, 13};

    unsigned long long result[R_SIZE] = {0};


    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }

    printf("Calculating Result...\n");
    start = __rdtsc();
    t_sqrt(p1, p2, result);
    if (result[0] == (unsigned long long)-1){
        printf("No square root exists.\n");
    } else {
        for(int i = 0;i<R_SIZE;i++){
        printf("%016llX\n", result[i]);
        }
    }
    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);

    return 0;
}
