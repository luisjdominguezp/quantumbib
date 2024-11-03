#include <openssl/evp.h>
#include <openssl/types.h>
#include <stdio.h>
#include <string.h>

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

int main() {
    const char *message = "------ This is SHA3-256 ------\n";
    //256 bits / 8 = 32 bytes
    unsigned char digest[32];

    hash_sha3_256((const unsigned char *)message, strlen(message), digest);
    printf("SHA3-256 Digest: ");
    for(int i=0;i<32;i++){
        printf("%02X", digest[i]);
    
    }
    printf("\n");

    return 0;
}
