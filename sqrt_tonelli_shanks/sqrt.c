#include <stdio.h>
#include <gmp.h>
#include <inttypes.h>
#include <x86intrin.h>

#define SIZE 4
#define R_SIZE 8
#pragma intrinsic(__rdtsc)
#define NTEST 100000
#define BIT_LIMIT 64
#define MOD_HEX "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF" 

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

void t_sqrt(unsigned long long p1[], unsigned long long p2[], unsigned long long r[]){
    mpz_t b1, f, p, result;
    mpz_inits(b1, f, p, result, NULL);

    mpz_import(b1, SIZE, 1, sizeof(unsigned long long), 0, 0, p1);
    mpz_import(p, SIZE, 1, sizeof(unsigned long long), 0, 0, p2);
    gmp_printf("Value of base: %ZX\n", b1);
    gmp_printf("Value of modulus p: %ZX\n", p);

    mpz_powm_ui(f, b1, 2, p);
    gmp_printf("Computed f = x^2 mod p: %ZX\n", f);

    // Check if f is a quadratic residue modulo p
    int legendre_value = legendre(f, p);
    if (legendre_value != 1) {
        for (int i = 0; i < R_SIZE; i++) {
            r[i] = 0;
        }
        r[0] = (unsigned long long)-1;
        mpz_clears(f, p, b1, result, NULL);
        return;
    }

    if (mpz_cmp_ui(p, 2) == 0) {
        mpz_set(result, f);
        size_t count;
        mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, result);
        mpz_clears(f, p, b1, result, NULL);
        return;
    }

    // Find Q and S such that p - 1 = Q * 2^S, with Q odd
    mpz_t Q, tmp;
    unsigned long S = 0;
    mpz_inits(Q, tmp, NULL);

    mpz_sub_ui(Q, p, 1); // Q = p - 1

    // Compute S and Q
    while (mpz_even_p(Q)) {
        mpz_fdiv_q_2exp(Q, Q, 1); // Q = Q / 2
        S++;
    }

    // Find a quadratic non-residue z modulo p
    mpz_t z;
    mpz_init_set_ui(z, 2);
    while (legendre(z, p) != -1) {
        mpz_add_ui(z, z, 1);
    }

    mpz_t c, x, t, b;
    mpz_inits(c, x, t, b, NULL);

    // c = z^Q mod p
    mpz_powm(c, z, Q, p);

    // x = f^((Q + 1) / 2) mod p
    mpz_add_ui(tmp, Q, 1);
    mpz_fdiv_q_2exp(tmp, tmp, 1); // tmp = (Q + 1) / 2
    mpz_powm(x, f, tmp, p);

    // t = f^Q mod p
    mpz_powm(t, f, Q, p);

    unsigned long M = S;

    while (mpz_cmp_ui(t, 1) != 0) {
        mpz_t t2;
        mpz_init(t2);
        unsigned long i = 1;
        mpz_powm_ui(t2, t, 2, p); // t2 = t^2 mod p

        while (i < M && mpz_cmp_ui(t2, 1) != 0) {
            mpz_powm_ui(t2, t2, 2, p); // t2 = t2^2 mod p
            i++;
        }
        mpz_clear(t2);

        if (i == M) {
            // No square root exists
            for (int idx = 0; idx < R_SIZE; idx++) {
                r[idx] = 0;
            }
            r[0] = (unsigned long long)-1;
            mpz_clears(f, p, b1, result, Q, tmp, z, c, x, t, b, NULL);
            return;
        }

        // b = c^(2^(M - i - 1)) mod p
        mpz_powm_ui(b, c, 1UL << (M - i - 1), p);

        // x = x * b mod p
        mpz_mul(x, x, b);
        mpz_mod(x, x, p);

        // t = t * b^2 mod p
        mpz_powm_ui(b, b, 2, p);
        mpz_mul(t, t, b);
        mpz_mod(t, t, p);

        //update c = b
        mpz_set(c, b);

        M = i;
    }

    mpz_set(result, x);

    size_t count;
    mpz_export(r, &count, 1, sizeof(unsigned long long), 0, 0, result);

    mpz_t verification;
    mpz_init(verification);
    mpz_powm_ui(verification, result, 2, p);

    if (mpz_cmp(verification, f) == 0) {
        printf("Verification succeeded: r^2 mod p == f\n");
    } else {
        printf("Verification failed: r^2 mod p != f\n");
    }

    mpz_clears(f, p, b1, result, Q, tmp, z, c, x, t, b, verification, NULL);
}

int main(){
    uint64_t start, end;
    int variable = 0;

    //unsigned long long p1[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    //unsigned long long p2[SIZE] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

    //unsigned long long p1[SIZE] = {0, 0, 0, 9};
    //FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF
    unsigned long long p2[SIZE] = {0xFFFFFFFF00000001, 0x0000000000000000, 0x00000000FFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    //unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF, 0x0000000000000000, 0xFFFFFFFF00000001};
    //unsigned long long p2[SIZE] = {0x1900000000000067, 0x175700000000004d, 0xe101d68000000016, 0x2523648240000002};
    
    //unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    unsigned long long p1[SIZE] = {0x00000000FFFFFFFE, 0xFFFFFFFF00000001, 0x0000000000000000, 0x0000000000000000};
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
