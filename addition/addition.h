#ifndef ADDITION_H
#define ADDITION_H

#include <stdio.h>
/**
 * @brief Adds two large numbers with carry.
 *
 * This function takes two arrays representing large numbers and adds them together,
 * storing the result in a third array. It handles carry-over between array elements.
 *
 * @param p1 Pointer to the first operand array.
 * @param p2 Pointer to the second operand array.
 * @param r Pointer to the result array where the sum will be stored.
 */

void add_with_carry(unsigned long long p1[], unsigned long long p2[], unsigned long long r[], int SIZE);

#endif // ADDITION_H
