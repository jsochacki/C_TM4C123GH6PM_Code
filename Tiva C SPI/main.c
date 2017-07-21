/*

Author: Mason Edgar
Supervisor: John Sochacki
ViaSat Inc. Tempe, AZ

Master Code For TivaC PLL Control

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
#include "inc/hw_uart.h"
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
#include "driverlib/ssi.h"
#include "Synth_API_Macro_Definitions.h"
#include "Synth_API_SPI_Setup.h"
#include "Synth_API_Console_Functions.h"
#include "Synth_API_Control_Registers.h"
#include "Synth_API_Status_Registers.h"


char new_line[] = "\n\r";

void DetermineFeedbackValues(float output_freq, float reference_freq, unsigned short* nINT, unsigned long* nFRAC, unsigned long nMOD);

int main(void) {
	

	char outputFreq[100], refFreq[100];
	float output, reference, result;

	setupClock();
	InitConsole();
	setupChipSelect(CS_ACTIVE_LOW);
	setupSPI_8Bit();



	while(1){

		clearArray(outputFreq);
		clearArray(refFreq);
		printString("\n\r\nEnter desired output frequency: ");
		getString(outputFreq);
		printString(new_line);
		printString("Enter the reference frequency: ");
		getString(refFreq);

		// Testing out my string to float conversion function
		output = ConvertStringToFloat(outputFreq);
		reference = ConvertStringToFloat(refFreq);

		result = output / reference;

		printString(new_line);
		printString(new_line);
		printFloat(result);
		printString(new_line);
		printString(new_line);


		/* Currently working on the command interface that parses user input and converts it to function arguments for register programming */



//		parseString(input, command, value);
//
//
//		if(!strncmp(command, "setFrequency", 12)){
//			printString("\n\r\n");
//			printString("Frequency set to ");
//			printString(value);
//			printString("\n\r\n");
//			printString("Enter command: ");
//		}
//
//		else if(!strncmp(command, "initDevice", 10)){
//			printString("\n\r\n");
//			printString("Initializing device...");
//			printString("\n\r\n");
//			printString("Enter command: ");
//		}
//
//		else{
//			printString("\n\r\n");
//			printString("Command not recognized...");
//			printString("\n\r\n");
//			printString("Enter command: ");
//
//		}




	}



	return 0;
}


// Part of command interface

void DetermineFeedbackValues(float output_freq, float reference_freq, unsigned short* nINT, unsigned long* nFRAC, unsigned long nMOD){

	float freq_ratio = output_freq / reference_freq;

	*nINT = (unsigned short)freq_ratio;

	float decimal_portion = freq_ratio - (unsigned short)freq_ratio;

	*nFRAC = decimal_portion * nMOD;

}
