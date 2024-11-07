#include <stdio.h>
#include <gmp.h>

void generate_quadratic_residue(mpz_t N, mpz_t g, mpz_t p) {
    mpz_powm_ui(N, g, 2, p); // N = g^2 mod p
}

int main() {
    mpz_t p, g, N;
    mpz_inits(p, g, N, NULL);

    // Initialize p to a large prime (you can use the previous p value)
    mpz_set_str(p, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBEAAEDCE6AF48A03BBFD25E8CD0364141", 16);
    
    // Set g to a random base value less than p
    mpz_set_ui(g, 5);  // You can choose any small integer

    // Generate N = g^2 mod p
    generate_quadratic_residue(N, g, p);

    gmp_printf("Generated quadratic residue N: %Zx\n", N);

    // Now you can use N as input for your square root function

    mpz_clears(p, g, N, NULL);
    return 0;
}

