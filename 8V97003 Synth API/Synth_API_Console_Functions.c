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

void InitConsole(void){

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

void clearArray(char* input){

	int i;

	for(i = 0; i < MAX_INPUT_LENGTH; i++){ *input++ = '\0'; }

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


/*******************************************************************************************************/
/*                    Converts a user input string into a floating point value                         */
/*******************************************************************************************************/

float ConvertStringToFloat(char* string) {

	// Converts a character array into a floating point number
	// This function can handle numbers with or without a decimal

	char integerPart[21], decimalPart[21];
	char* int_ptr = integerPart;
	char* dec_ptr = decimalPart;
	char* decimal_check = string;
	int i, flag = 0, decimal_flag = 0;
	unsigned long integer_portion, decimal_portion;
	float mult = 0, divider = 1, result;
	float intPart, decPart;

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

		intPart = (float)integer_portion;
		decPart = (float)decimal_portion;

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
		intPart = (float)integer_portion;

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