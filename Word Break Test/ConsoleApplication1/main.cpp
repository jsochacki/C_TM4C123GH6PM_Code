#include <stdio.h>
#include <stdlib.h>


#define LOWER_8_BITS 0x000000FF
#define MIDDLE_8_BITS 0X0000FF00
#define UPPER_8_BITS 0x00FF0000

int main(void)
{

	unsigned long temp = 0x128565; 

	
	unsigned long num1 = temp & UPPER_8_BITS;
	unsigned long num2 = temp & LOWER_8_BITS;
	unsigned long num3 = temp & MIDDLE_8_BITS; 

	unsigned short upper = num1 >> 16;
	unsigned short middle = num3 >> 8;
	unsigned short lower = num2; 

	


	printf("upper: %x\n\nmiddle: %x\n\nlower: %x\n\n", upper, middle, lower);









	return 0;
}
