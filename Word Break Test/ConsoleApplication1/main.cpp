#include <stdio.h>
#include <stdlib.h>


#define LOWER_8_BITS 0x000000FF
#define MIDDLE_8_BITS 0X0000FF00
#define UPPER_8_BITS 0x00FF0000
#define temp 0x987654
#define TOP_HALF_16_BIT 0xFF00
#define BOTTOM_HALF_16_BIT 0x00FF


void foo(unsigned long word);

int main(void)
{

	unsigned short val = 4660; 

	unsigned short top = val & TOP_HALF_16_BIT;
	unsigned short bottom = val & BOTTOM_HALF_16_BIT; 

	top = top >> 8;

	printf("top: %x\n\nbottom: %x\n\ndecimal: %d\n\n", top, bottom, (top<<8) | bottom);





	return 0;
}


void foo(unsigned long word) {

	unsigned long num1 = word & UPPER_8_BITS;
	unsigned long num2 = word & LOWER_8_BITS;
	unsigned long num3 = word & MIDDLE_8_BITS;

	unsigned short upper = num1 >> 16;
	unsigned short middle = num3 >> 8;
	unsigned short lower = num2;

	printf("upper: %x\n\nmiddle: %x\n\nlower: %x\n\n", upper, middle, lower);

}