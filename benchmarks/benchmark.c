#include <criterion/criterion.h>
#include <criterion/benchmark.h>
#include "addition/addition.h"
#include "multiplication/multiplication.h"

#define SIZE 4
unsigned long long p1[SIZE] = {0xFFFFFFFFFFFFFFFF, 0x0, 0x123456789ABCDEF0, 0x9876543210FEDCBA};
unsigned long long p2[SIZE] = {0x1111111111111111, 0x2222222222222222, 0x3333333333333333, 0x4444444444444444};
unsigned long long result[SIZE] = {0};

// Benchmark the addition function
Benchmark(addition, add_with_carry) {
    add_with_carry(p1, p2, result, SIZE);
}

// Benchmark the multiplication function
Benchmark(multiplication, mult) {
    mult(p1, p2, result, SIZE, SIZE * 2);
}

