#include <criterion/criterion.h>
#include "../addition/addition.h"
#include "../multiplication/multiplication.h"

#define SIZE 4
#define R_SIZE 8

Test(addition, add_with_carry_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
    unsigned long long p2[SIZE] = {0x1111111111111111, 0x2222222222222222, 0x3333333333333333, 0x4444444444444444};
    unsigned long long result[SIZE] = {0};

    add_with_carry(p1, p2, result, SIZE);
}

Test(multiplication, mult_benchmark) {
    unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    unsigned long long p2[SIZE] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF}; 
    unsigned long long result[R_SIZE] = {0};

    mult(p1, p2, result, SIZE, R_SIZE);
}

