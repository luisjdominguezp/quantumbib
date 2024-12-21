#include <openssl/evp.h>
#include <openssl/types.h>
#include <stdint.h>
#include <stdio.h>
#include <x86intrin.h>
#include <string.h>
/*
#pragma intrinsic(__rdtsc)
#define NTEST 100000

void measured_function(volatile int *var) {(*var) = 1; }
*/
// Copyright © 2024 Horacio Hernandez
void hash_sha3_256(const unsigned char *data, size_t len, unsigned char *digest){
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "Error initializing EVP_MD_CTX\n");
        return;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha3_256(), NULL) != 1) {
        fprintf(stderr, "Error initializing SHA3-256 digest\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        fprintf(stderr, "Error updating SHA3-256 digest\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    if (EVP_DigestFinal_ex(ctx, digest, NULL) != 1) {
        fprintf(stderr, "Error finalizing SHA3-256 digest\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    EVP_MD_CTX_free(ctx);
}
/*
int main() {
    uint64_t start, end;
    int variable = 0;
    const char *message = "message to be hashed\n";
    //256 bits / 8 = 32 bytes
    unsigned char digest[32];

    printf("Warming up the cpu.\n");
    for (int i = 0;i<NTEST;i++){
        measured_function(&variable);
    }
    printf("Calculating Result...\n");
    start = __rdtsc();
    for(int i =0;i<NTEST;i++){
        hash_sha3_256((const unsigned char *)message, strlen(message), digest);
    }
    printf("SHA3-256 Digest: ");
    for(int i=0;i<32;i++){
        printf("%02X", digest[i]);
    
    }
    printf("\n");

    end = __rdtsc();

    printf("Total = %f CPU cycles\n", (float)(end - start) / NTEST);
    return 0;

}
*/
