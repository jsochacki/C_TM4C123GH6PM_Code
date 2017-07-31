/*
 * Synth_API_Console_Functions.c
 *
 * Author: Mason Edgar
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_adc.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pwm.h"
#include "driverlib/ssi.h"
#include "Synth_API_Macro_Definitions.h"
#include "Synth_API_Console_Functions.h"




/*******************************************************************************************************/
/*                           Initializes the console using the UART0 module                            */
/*******************************************************************************************************/

void initConsole(void){

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
	MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
	MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	MAP_UARTConfigSetExpClk(UART0_BASE, MAP_SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	printString("Welcome to the 8V97003 API Console\n\r");
	printString("\nEnter a command: ");


}


/*******************************************************************************************************/
/*           Clears out a given character array with null terminators for housekeeping                 */
/*******************************************************************************************************/

void clearArray(char* input, int sizeOfArray){

	int i;

	for(i = 0; i < sizeOfArray; i++){ *input++ = '\0'; }

}


/*******************************************************************************************************/
/*                     Gets a string from the UART console entered by a user                           */
/*******************************************************************************************************/

void getString(char* user_string){

	int count = 0;

	while( (*user_string = MAP_UARTCharGet(UART0_BASE)) != ENTER ){

		MAP_UARTCharPut(UART0_BASE, *user_string++);
		if(++count == MAX_INPUT_LENGTH){ break; }

	}

}


/*******************************************************************************************************/
/*               Parses the user input and separates the command from the value                        */
/*******************************************************************************************************/

void parseString(char* original, char* command, char* value){

	int flag = 0;

	while(*original != '\0'){

		while(*original != ' ' && flag == 0){

			if(*original == '\0'){ break; }

			*command = *original++;
			 command++;
		}

		if(*original == ' '){
			flag = 1;
			original++;
		}

		if(*original == '\0'){ break; }

		*value = *original++;
		 value++;

	}

}


/*******************************************************************************************************/
/*                           Prints a given string to the UART console                                 */
/*******************************************************************************************************/

void printString(char *string){

    uint32_t ui32Counter = 0;

    while (string[ui32Counter] != '\0'){

        MAP_UARTCharPut(UART0_BASE, string[ui32Counter]);
        ui32Counter = ui32Counter + 1;
    }
}


/**********************************************************************************************************************************/
/*                                         Converts a string into a frequency value                                               */
/**********************************************************************************************************************************/

double ConvertStringToFrequency(char* string, int* factor) {

	// The frequency range will typically be GHz, so I wanted to avoid using large numbers since they are prone to rounding errors

	char value[20], unit[20];
	char* value_ptr = value;
	char* unit_ptr = unit;
	int i;

	for (i = 0; i < 20; i++) {
		value[i] = '\0';
		unit[i] = '\0';
	}

	// Copy number portion into separate array (58 is the limit of the numbers inside the ASCII table)
	while (*string < 58) {
		*value_ptr = *string++;
		value_ptr++;
	}

	while (*string != ENTER) {
		*unit_ptr = *string++;
		unit_ptr++;
	}

	double value_flt = ConvertStringToFloat(value);

	if (!strncmp(unit, "GHz", 3)) {
		*factor = 9; // 10^9 power
	}
	else if (!strncmp(unit, "MHz", 3)) {
		*factor = 6; // 10^6 power
	}
	else if (!strncmp(unit, "kHz", 3)) {
		*factor = 3; // 10^3 power
	}
	else if (!strncmp(unit, "Hz", 3)) {
		*factor = 0; // 10^0 power
	}

	return value_flt;
}


/**********************************************************************************************************/
/*                   Generates the proper frequency ratio for feedback divider control                    */
/**********************************************************************************************************/

double GenerateFrequencyRatio(double output_freq, int out_factor, double reference_freq, int ref_factor) {

	double ratio;
	int i;

	int result_factor = out_factor - ref_factor; // Determine what the difference in order of magnitude is between the output and reference

	// If the reference is larger, switch it up
	if (result_factor < 0) {

		result_factor = ref_factor - out_factor;

		double multiplier = 1;

		for (i = 0; i < result_factor; i++) { multiplier *= 10; }

		output_freq /= multiplier;

		ratio = output_freq / reference_freq;
	}

	else {

		double multiplier = 1;

		for (i = 0; i < result_factor; i++) { multiplier *= 10; }

		output_freq *= multiplier;

		ratio = output_freq / reference_freq;
	}

	return ratio;
}


/*******************************************************************************************************/
/*                       Converts a character array into a boolean value                               */
/*******************************************************************************************************/

int ConvertStringToBool(char* string) {

	if (!strncmp(string, "y", 1)) {
		return 1;
	}
	else if (!strncmp(string, "n", 1)) {
		return 0;
	}

}

/*******************************************************************************************************/
/*                    Converts a character array into a floating point value                           */
/*******************************************************************************************************/

double ConvertStringToFloat(char* string) {

	// The difference between this function and the one below it is how this function determines the end of the character arrays

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


/*******************************************************************************************************/
/*                    Converts a user input string into a floating point value                         */
/*******************************************************************************************************/

double ConvertUserStringToFloat(char* string) {

	// Converts a character array into a floating point number
	// This function can handle numbers with or without a decimal

	char integerPart[21], decimalPart[21];
	char* int_ptr = integerPart;
	char* dec_ptr = decimalPart;
	char* decimal_check = string;
	int i, flag = 0, decimal_flag = 0;
	unsigned long integer_portion, decimal_portion;
	double mult = 0, divider = 1, result;
	double intPart, decPart;

	// Clear out arrays
	for (i = 0; i < 20; i++) {

		integerPart[i] = '\0';
		decimalPart[i] = '\0';
	}

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
		while (*string != ENTER) {

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

		result = intPart + decPart;
	}

	else {

		// Copy integer portion (no decimal place)
		while (*string != ENTER) {
			*int_ptr = *string++;
			 int_ptr++;
		}

		integer_portion = ConvertStringToNumber(integerPart);
		intPart = (double)integer_portion;

		result = intPart;
	}

	return result;
}


/*************************************************************************************************************/
/*           Takes a character string entered by a user and converts it to an unsigned long int              */
/*************************************************************************************************************/

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

/*******************************************************************************************************/
/*            Converts a given float value to a character array and prints to the console              */
/*******************************************************************************************************/

void printFloat(float value){

    printInt(value);

    int subtract = value;

    // 1 decimal place per zero
    value = (value - subtract) * 100000000;
    MAP_UARTCharPut(UART0_BASE, '.');

    if(value < 0)
        value = -value;

    printInt(value);

}

/*******************************************************************************************************/
/*          Converts a given integer value to a character array and prints to the console              */
/*******************************************************************************************************/

void printInt(int value){

    int count = 0;

    int num[8];

    if(value < 0){
        value = -value;
        MAP_UARTCharPut(UART0_BASE, '-');
    }

    int temp = value;

    while(temp >= 10){

        num[count] = temp % 10;
        count++;
        temp = temp * .1;
    }

    num[count] = temp % 10;

        int i;

        for(i = count; i >= 0; i--){
            printSingleNumber(num[i]);
        }

}


/*******************************************************************************************************/
/*                       Converts a single digit to a single ASCII character                           */
/*******************************************************************************************************/

void printSingleNumber(int num){

    switch (num) {
    case 0:
        MAP_UARTCharPut(UART0_BASE, '0');
        break;
    case 1:
        MAP_UARTCharPut(UART0_BASE, '1');
        break;
    case 2:
        MAP_UARTCharPut(UART0_BASE, '2');
        break;
    case 3:
        MAP_UARTCharPut(UART0_BASE, '3');
        break;
    case 4:
        MAP_UARTCharPut(UART0_BASE, '4');
        break;
    case 5:
        MAP_UARTCharPut(UART0_BASE, '5');
        break;
    case 6:
        MAP_UARTCharPut(UART0_BASE, '6');
        break;
    case 7:
        MAP_UARTCharPut(UART0_BASE, '7');
        break;
    case 8:
        MAP_UARTCharPut(UART0_BASE, '8');
        break;
    case 9:
        MAP_UARTCharPut(UART0_BASE, '9');
        break;
    default:
        MAP_UARTCharPut(UART0_BASE, 'E');
      break;
    }
}
