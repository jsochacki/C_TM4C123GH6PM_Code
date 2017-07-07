#include <stdio.h>
#include <stdlib.h>


#define LOWER_8_BITS 0x000000FF
#define MIDDLE_8_BITS 0X0000FF00
#define UPPER_8_BITS 0x00FF0000
#define temp 0x987654
#define TOP_HALF_16_BIT 0xFF00
#define BOTTOM_HALF_16_BIT 0x00FF
#define TOP_8_32Bit 0xFF000000
#define UPPER_MIDDLE_32Bit 0x00FF0000
#define LOWER_MIDDLE_32Bit 0x0000FF00
#define BOTTOM_8_32Bit 0x000000FF


#define REG_10h 0x1000
#define REG_11h 0x1100
#define REG_12h 0x1200
#define REG_13h 0x1300
#define REG_14h 0x1400



void foo(unsigned long word);
void AddNum(int a, int b, int* c);
void SplitNumber_16Bit(unsigned short original, unsigned long* top_half, unsigned long* bottom_half);
unsigned long Create24BitWord(unsigned long data, unsigned long reg);
void SplitNumber_32Bit(unsigned long original, unsigned long* top_portion, unsigned long* upper_middle_portion, unsigned long* lower_middle_portion, unsigned long* bottom_portion);

int main(void)
{
	/*
	unsigned short val = 4660; 

	unsigned short top = val & TOP_HALF_16_BIT;
	unsigned short bottom = val & BOTTOM_HALF_16_BIT; 

	top = top >> 8;

	printf("top: %x\n\nbottom: %x\n\ndecimal: %d\n\n", top, bottom, (top<<8) | bottom);
	

	int a = 5; 
	int b = 3; 
	int c; 

	AddNum(a, b, &c); */


	//unsigned short Nint = 0x1234; 
	//unsigned long top_half, bottom_half; 

	//SplitNumber_16Bit(Nint, &top_half, &bottom_half);

	//printf("Top Half: %x\nBottom half: %x\n\n", top_half, bottom_half); 

	//unsigned long word; 

	//word = Create24BitWord(bottom_half, REG_10h);

	//printf("Word: %x\n\n", word); 

	unsigned long test = 0x12345678; 
	unsigned long upper, upper_middle, lower_middle, bottom; 

	SplitNumber_32Bit(test, &upper, &upper_middle, &lower_middle, &bottom); 

	printf("Top: %x\nUpper Middle: %x\nLower Middle: %x\nBottom: %x\n", upper, upper_middle, lower_middle, bottom);








	return 0;
}


unsigned long Create24BitWord(unsigned long data, unsigned long reg){

	unsigned long word; 

	word = data | reg; 

	return word;

}







void SplitNumber_16Bit(unsigned short original, unsigned long* top_half, unsigned long* bottom_half) {

	unsigned short top = original & TOP_HALF_16_BIT;
	unsigned short bottom = original & BOTTOM_HALF_16_BIT;

	unsigned long top_32bit = top >> 8;
	unsigned long bottom_32bit = bottom;

	*top_half = top_32bit; 
	*bottom_half = bottom_32bit; 
}


void SplitNumber_32Bit(unsigned long original, unsigned long* top_portion, unsigned long* upper_middle_portion, unsigned long* lower_middle_portion, unsigned long* bottom_portion){

	unsigned long top = original & TOP_8_32Bit;
	unsigned long upper_middle = original & UPPER_MIDDLE_32Bit;
	unsigned long lower_middle = original & LOWER_MIDDLE_32Bit; 
	unsigned long bottom = original & BOTTOM_8_32Bit;

	top = top >> 24;
	upper_middle = upper_middle >> 16;
	lower_middle = lower_middle >> 8;

	*top_portion = top;
	*upper_middle_portion = upper_middle; 
	*lower_middle_portion = lower_middle;
	*bottom_portion = bottom;
}



void AddNum(int a, int b, int* c) {

	*c = a + b; 

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