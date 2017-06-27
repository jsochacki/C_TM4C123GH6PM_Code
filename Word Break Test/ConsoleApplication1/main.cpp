#include <stdio.h>
#include <stdlib.h>


#define UPPER_16_BITS 0xFFFF0000
#define LOWER_16_BITS 0x0000FFFF

int main(void)
{

	unsigned long temp = 0x12345678; 

	unsigned long num1 = temp & UPPER_16_BITS;
	unsigned long num2 = temp & LOWER_16_BITS;

	unsigned short upper = num1 >> 16; 
	unsigned short lower = num2; 

	


	printf("upper: %x\n\nlower: %x\n\n", upper, lower);









	return 0;
}
