#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdint.h>
#include <stdbool.h>


//void foo(unsigned long word);
//void AddNum(int a, int b, int* c);
//void SplitNumber_16Bit(unsigned short original, unsigned long* top_half, unsigned long* bottom_half);
//unsigned long Create24BitWord(unsigned long data, unsigned long reg);
//void SplitNumber_32Bit(unsigned long original, unsigned long* top_portion, unsigned long* upper_middle_portion, unsigned long* lower_middle_portion, unsigned long* bottom_portion);

unsigned long ConvertStringToNumber(char* string);
double ConvertStringToFloat(char* string);
float ConvertUserStringToFloat(char* string);
void MaximizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD);
void MinimizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD);
void DetermineFeedbackValues(double freq_ratio, unsigned short* nINT, unsigned long* nFRAC, unsigned long* nMOD);
double ConvertStringToFrequency(char* string, int* factor);
double GenerateFrequencyRatio(double output_freq, int out_factor, double reference_freq, int ref_factor);
int StringToBool(char* string);
unsigned long ConvertStringToPowerSetting(char* string);
unsigned long ConvertStringToChargePumpCurrent(char* string, bool NMOS_Current, char* delta);
unsigned long ConvertStringToBleederCurrent(char* string);
void OptimizeReferenceForPhaseNoise(double freqRatio, unsigned long* Mult, unsigned long* R_divider);


#define SHIFT_AMOUNT 16 // 2^16 = 65536

const int fractionMask = 0xFFFFFFFFFFFFFFFF >> (64 - SHIFT_AMOUNT);

#define DoubleToFixed(x) ((x) * (double)((uint64_t)1 << SHIFT_AMOUNT))
#define FixedToDouble(x) ((double)(x) / (double)((uint64_t)1 << SHIFT_AMOUNT))
#define FractionPart(x) ((x) & fractionMask)
#define FixedMultiply(x,y) (((x) * (y)) >> SHIFT_AMOUNT)
#define FixedDivide(x,y) (((x) << SHIFT_AMOUNT) / (y))


int main(void)
{
	char pfd[20], ref[20];
	int pfdFactor, refFactor;
	unsigned long MultValue, R_Divider;

	printf("Enter desired PFD frequency [MHz]: ");
	fgets(pfd, sizeof(pfd), stdin);
	printf("\nEnter external reference frequency [MHz]: ");
	fgets(ref, sizeof(ref), stdin);

	double PFDFreq = ConvertStringToFrequency(pfd, &pfdFactor);
	double refFreq = ConvertStringToFrequency(ref, &refFactor);
	
	double result = GenerateFrequencyRatio(PFDFreq, pfdFactor, refFreq, refFactor);

	printf("\nResult: %f\n", result);

	OptimizeReferenceForPhaseNoise(result, &MultValue, &R_Divider);

	printf("Multiplier Value: %u\n\nR Divider Value: %u\n\n", MultValue, R_Divider);
	
	return 0;
}

void OptimizeReferenceForPhaseNoise(double freqRatio, unsigned long* Mult, unsigned long* R_divider) {

	double MultValue = 2, RefDoubler = 2, increment = 1;
	double realRDividerValue;

	uint64_t freqRatioFixed = DoubleToFixed(freqRatio);
	uint64_t MultValueFixed = DoubleToFixed(MultValue);
	uint64_t RefDoublerFixed = DoubleToFixed(RefDoubler);
	uint64_t IncFixed = DoubleToFixed(increment);

	uint64_t RDividerFixed = FixedDivide(FixedMultiply(RefDoublerFixed, MultValueFixed), freqRatioFixed);

	realRDividerValue = FixedToDouble(RDividerFixed);
	
	while (realRDividerValue - (unsigned long)realRDividerValue != 0) {
		MultValueFixed = MultValueFixed + IncFixed;
		RDividerFixed = FixedDivide(FixedMultiply(RefDoublerFixed, MultValueFixed), freqRatioFixed);
		realRDividerValue = FixedToDouble(RDividerFixed);
	 }

	MultValue = FixedToDouble(MultValueFixed);

	*Mult = (unsigned long)MultValue;
	*R_divider = (unsigned long)realRDividerValue;

}








unsigned long ConvertStringToBleederCurrent(char* string) {

	double current_uA = ConvertUserStringToFloat(string);

	double value = (current_uA / 5.33);

	value = round(value);

	return (unsigned long)value;
}


unsigned long ConvertStringToChargePumpCurrent(char* string, bool NMOS_Current, char* delta) {

	double delta_value; 

	double current_mA = ConvertUserStringToFloat(string);

	if (NMOS_Current) { delta_value = ConvertUserStringToFloat(delta); }
	else { delta_value = 0; }

	current_mA -= delta_value;

	double value = (current_mA / (166.66E-6 * 1000)) - 1;

	value = round(value);

	return (unsigned long)value;
}


unsigned long ConvertStringToPowerSetting(char* string) {

	if (!strncmp(string, "-2dBm", 5)) { return 1; }
	else if (!strncmp(string, "0dBm", 4)) { return 3; }
	else if (!strncmp(string, "+0dBm", 5)) { return 3; }
	else if (!strncmp(string, "+2dBm", 5)) { return 7; }
	else if (!strncmp(string, "+4dBm", 5)) { return 15; }
	else if (!strncmp(string, "+6dBm", 5)) { return 31; }
	else if (!strncmp(string, "+8dBm", 5)) { return 63; }
	else if (!strncmp(string, "+10dBm", 6)) { return 127; }
	else if (!strncmp(string, "+12dBm", 6)) { return 255; }

}

int StringToBool(char* string) {
	
	if (!strncmp(string, "y", 1)) {
		return 1;
	}
	else if (!strncmp(string, "n", 1)) {
		return 0;
	}

}


double GenerateFrequencyRatio(double output_freq, int out_factor, double reference_freq, int ref_factor) {

	double ratio; 

	int result_factor = out_factor - ref_factor;

	if (result_factor < 0) {

		result_factor = ref_factor - out_factor; 

		double multiplier = 1;

		for (int i = 0; i < result_factor; i++) { multiplier *= 10; }

		output_freq /= multiplier;

		ratio = output_freq / reference_freq;
	}

	else {

		double multiplier = 1;

		for (int i = 0; i < result_factor; i++) { multiplier *= 10; }

		output_freq *= multiplier;

		ratio = output_freq / reference_freq;
	}

	return ratio;
}

double ConvertStringToFrequency(char* string, int* factor) {

	char value[20], unit[20]; 
	char* value_ptr = value; 
	char* unit_ptr = unit;

	for (int i = 0; i < 20; i++) {
		value[i] = '\0';
		unit[i] = '\0';
	}

	// Copy number portion into separate array (58 is the limit of the numbers inside the ASCII table)
	while (*string < 58) {
		*value_ptr = *string++; 
		value_ptr++;
	}

	while (*string != '\n') {
		*unit_ptr = *string++;
		unit_ptr++;
	}

	double value_flt = ConvertStringToFloat(value);
	
	if (!strncmp(unit, "GHz", 3)) {
		*factor = 9;
	}
	else if (!strncmp(unit, "MHz", 3)) {
		*factor = 6;
	}
	else if (!strncmp(unit, "kHz", 3)) {
		*factor = 3;
	}
	else if (!strncmp(unit, "Hz", 3)) {
		*factor = 0;
	}

	return value_flt;
}

void FixedPointMinimizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD) {

	double modulus_min = 2, increment = 1;

	uint64_t IncFixed = DoubleToFixed(increment);
	uint64_t fixed_mod_min = DoubleToFixed(modulus_min);
	uint64_t fixed_ratio = DoubleToFixed(ratio);
	uint64_t fixed_frac = FixedMultiply(fixed_ratio, fixed_mod_min);

	double real_frac = FixedToDouble(fixed_frac);

	while ((real_frac - (unsigned long)real_frac) != 0) {
		fixed_mod_min = fixed_mod_min + IncFixed;
		fixed_frac = FixedMultiply(fixed_ratio, fixed_mod_min);
		real_frac = FixedToDouble(fixed_frac);
	}

	double real_mod_min = FixedToDouble(fixed_mod_min);

	*nFRAC = (unsigned long)real_frac;
	*nMOD = (unsigned long)real_mod_min;

}













void FixedPointMaximizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD) {

	double modulus_max = 4294967295, increment = 1;

	uint64_t IncFixed = DoubleToFixed(increment);
	uint64_t fixed_mod_max = DoubleToFixed(modulus_max);
	uint64_t fixed_ratio = DoubleToFixed(ratio);
	uint64_t fixed_frac = FixedMultiply(fixed_ratio, fixed_mod_max);

	double real_frac = FixedToDouble(fixed_frac);

	while ((real_frac - (unsigned long)real_frac) != 0) {
		fixed_mod_max = fixed_mod_max - IncFixed;
		fixed_frac = FixedMultiply(fixed_ratio, fixed_mod_max);
		real_frac = FixedToDouble(fixed_frac);
	}

	double real_mod_max = FixedToDouble(fixed_mod_max);

	*nFRAC = (unsigned long)real_frac;
	*nMOD = (unsigned long)real_mod_max;
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



void DetermineFeedbackValues(double freq_ratio, unsigned short* nINT, unsigned long* nFRAC, unsigned long* nMOD) {

	unsigned long frac, mod;

	*nINT = (unsigned short)freq_ratio;

	double decimal_portion = freq_ratio - (unsigned short)freq_ratio;

	FixedPointMaximizeMOD(decimal_portion, &frac, &mod); // Choose whether you want to maximize the modulus or minimize it

	*nFRAC = frac;
	*nMOD = mod;

}


double ConvertStringToFloat(char* string) {

	char integerPart[20], decimalPart[21];
	char* int_ptr = integerPart;
	char* dec_ptr = decimalPart;
	char* decimal_check = string;
	char* no_decimal = string;
	int i, flag = 0, decimal_flag = 0;
	unsigned long integer_portion, decimal_portion;
	double mult = 0, divider = 1, result;
	double intPart, decPart;

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
		while (*string != '\0') {

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

		intPart = (double)integer_portion;
		decPart = (double)decimal_portion;

		for (i = 0; i < mult; i++) { divider *= 10; }

		decPart /= divider;

		//printf("\nnew dec part: %f\n", decPart);

		result = intPart + decPart;
	}

	else {

		// Copy integer portion (no decimal place)
		while (*string != '\0') {
			*int_ptr = *string++;
			int_ptr++;
		}

		integer_portion = ConvertStringToNumber(integerPart);
		intPart = (double)integer_portion;

		result = intPart;
	}

	return result;
}











float ConvertUserStringToFloat(char* string) { 

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



