#include <stdio.h>
#define SIZE 4
void add_with_carry(long long p1[], long long p2[], long long r[]){
    long long carry = 0;
    for(int i = 0; i<SIZE;i++){
        //I'm carrying any leftover overflow to the second number 
        //and then to the first number
        unsigned long long tmpSum = p2[i] + carry;
        r[i] = p1[i] + tmpSum;
        //if both are true carry = 1
        //basically meaning there was an overflow
        carry = (p1[i] > r[i]) | (p2[i] > tmpSum);
    }

} 

int main(){
    long long p1[SIZE] = {1, 2, 3, 4};
    long long p2[SIZE]  = {1, 2, 3, 4};

    long long result[SIZE] = {0};

    add_with_carry(p1, p2, result);

    printf("Result: ");
    for(int i = 0;i<SIZE;i++){
        printf("%llu ", result[i]);
    }
    printf("End of result.\n");

    return 0;
}
