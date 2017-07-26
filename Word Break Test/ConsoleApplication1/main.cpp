#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>



//void foo(unsigned long word);
//void AddNum(int a, int b, int* c);
//void SplitNumber_16Bit(unsigned short original, unsigned long* top_half, unsigned long* bottom_half);
//unsigned long Create24BitWord(unsigned long data, unsigned long reg);
//void SplitNumber_32Bit(unsigned long original, unsigned long* top_portion, unsigned long* upper_middle_portion, unsigned long* lower_middle_portion, unsigned long* bottom_portion);

unsigned long ConvertStringToNumber(char* string);
float ConvertStringToFloat(char* string);
void MaximizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD);
void MinimizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD);
void DetermineFeedbackValues(double output_freq, double reference_freq, unsigned short* nINT, unsigned long* nFRAC, unsigned long* nMOD);

int main(void)
{
	unsigned short integerPart;
	unsigned long fractionalPart, modPart;

	DetermineFeedbackValues(2000000, 4000000, &integerPart, &fractionalPart, &modPart);

	printf("nINT: %u\n", integerPart);
	printf("nFrac: %u\n", fractionalPart);
	printf("nMOD:  %u\n\n", modPart);

	return 0;
}


void MaximizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD) {

	double modulus_max = 4294967295;

	double frac = ratio * modulus_max;

	while ((frac - (unsigned long)frac) != 0) {
		modulus_max = modulus_max - 1;
		frac = ratio * modulus_max;
	}

	printf("frac: %f\n", frac);

	*nFRAC = (unsigned long)frac;
	*nMOD = (unsigned long)modulus_max;
} 



void MinimizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD) {

	double modulus_min = 2;

	double frac = ratio * modulus_min;

	while ((frac - (unsigned long)frac) != 0) {
		modulus_min = modulus_min + 1;
		frac = ratio * modulus_min;
	}

	printf("frac: %f\n", frac);

	*nFRAC = (unsigned long)frac;
	*nMOD = (unsigned long)modulus_min;

}



void DetermineFeedbackValues(double output_freq, double reference_freq, unsigned short* nINT, unsigned long* nFRAC, unsigned long* nMOD) {

	double freq_ratio = output_freq / reference_freq;
	unsigned long frac, mod;

	*nINT = (unsigned short)freq_ratio;

	double decimal_portion = freq_ratio - (unsigned short)freq_ratio;

	MaximizeMOD(decimal_portion, &frac, &mod);

	//	MinimizeMOD(decimal_portion, &frac, &mod);

	*nFRAC = frac;
	*nMOD = mod;

}


























float ConvertStringToFloat(char* string) { 

	char integerPart[20], decimalPart[21];
	char* int_ptr = integerPart;
	char* dec_ptr = decimalPart;
	char* decimal_check = string;
	char* no_decimal = string;
	int i, flag = 0, decimal_flag = 0;
	unsigned long integer_portion, decimal_portion;
	float mult = 0, divider = 1, result;
	float intPart, decPart;

	// Clear out arrays
	for (i = 0; i < 20; i++) { integerPart[i] = '\0'; }
	for (i = 0; i < 21; i++) { decimalPart[i] = '\0'; }

	// Check for a decimal
	while (*decimal_check != '\0') { 

		if (*decimal_check++ == '.') { decimal_flag = 1; } 
	}

	if (decimal_flag) {

		// Copy integer portion into workspace
		while (*string != '.') {
			*int_ptr = *string++;
			int_ptr++;
		}

		// Copy decimal portion into workspace
		while (*string != '\n') {

			if (flag == 0) {
				string++;
				flag = 1;
			}
			else {
				*dec_ptr = *string++;
				 dec_ptr++;
				 mult++;
			}
		}

		integer_portion = ConvertStringToNumber(integerPart);
		decimal_portion = ConvertStringToNumber(decimalPart);

		intPart = (float)integer_portion;
		decPart = (float)decimal_portion;

		for (i = 0; i < mult; i++) { divider *= 10; }

		decPart /= divider;

		result = intPart + decPart;
	}

	else {

		// Copy integer portion (no decimal place)
		while (*string != '\n') {
			*int_ptr = *string++;
			 int_ptr++;
		}

		integer_portion = ConvertStringToNumber(integerPart);
		intPart = (float)integer_portion;

		result = intPart;
	}

	return result;
}


unsigned long ConvertStringToNumber(char* string) {

	// This function properly converts all legal values for an >>unsigned<< long integer: 0d - 4,294,967,295d

	// Number string MUST be in decimal format, this function isn't designed to accept hex values 

	int i, digit, current_power = -1; 
	unsigned long value = 0;
	char* temp_ptr; 

	temp_ptr = string; 

	while (*temp_ptr++ != '\0') { current_power++; }

	while (*string != '\0') {

		unsigned long multiplier = 1; 

		for(i = current_power--; i > 0; i--) { multiplier *= 10; }
		
		switch (*string++) {

		case '0':
			digit = 0; 
			break;
		case '1': 
			digit = 1;
			break;
		case '2':
			digit = 2;
			break;
		case '3': 
			digit = 3;
			break;
		case '4':
			digit = 4;
			break;
		case '5':
			digit = 5;
			break;
		case '6':
			digit = 6;
			break;
		case '7':
			digit = 7;
			break;
		case '8':
			digit = 8;
			break;
		case '9':
			digit = 9;
			break;
		}

		value += digit * multiplier; 
	}

	return value;
}



//unsigned long Create24BitWord(unsigned long data, unsigned long reg){
//
//	unsigned long word; 
//
//	word = data | reg; 
//
//	return word;
//
//}
//
//
//
//
//
//
//
//void SplitNumber_16Bit(unsigned short original, unsigned long* top_half, unsigned long* bottom_half) {
//
//	unsigned short top = original & TOP_HALF_16_BIT;
//	unsigned short bottom = original & BOTTOM_HALF_16_BIT;
//
//	unsigned long top_32bit = top >> 8;
//	unsigned long bottom_32bit = bottom;
//
//	*top_half = top_32bit; 
//	*bottom_half = bottom_32bit; 
//}
//
//
//void SplitNumber_32Bit(unsigned long original, unsigned long* top_portion, unsigned long* upper_middle_portion, unsigned long* lower_middle_portion, unsigned long* bottom_portion){
//
//	unsigned long top = original & TOP_8_32Bit;
//	unsigned long upper_middle = original & UPPER_MIDDLE_32Bit;
//	unsigned long lower_middle = original & LOWER_MIDDLE_32Bit; 
//	unsigned long bottom = original & BOTTOM_8_32Bit;
//
//	top = top >> 24;
//	upper_middle = upper_middle >> 16;
//	lower_middle = lower_middle >> 8;
//
//	*top_portion = top;
//	*upper_middle_portion = upper_middle; 
//	*lower_middle_portion = lower_middle;
//	*bottom_portion = bottom;
//}
//
//
//
//void AddNum(int a, int b, int* c) {
//
//	*c = a + b; 
//
//}
//
//
//
//void foo(unsigned long word) {
//
//	unsigned long num1 = word & UPPER_8_BITS;
//	unsigned long num2 = word & LOWER_8_BITS;
//	unsigned long num3 = word & MIDDLE_8_BITS;
//
//	unsigned short upper = num1 >> 16;
//	unsigned short middle = num3 >> 8;
//	unsigned short lower = num2;
//
//	printf("upper: %x\n\nmiddle: %x\n\nlower: %x\n\n", upper, middle, lower);
//
//} 



