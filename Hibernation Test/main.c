#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
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
#include "driverlib/hibernate.h"
#include "Synth_API_Macro_Definitions.h"
#include "Synth_API_SPI_Setup.h"
#include "Synth_API_Console_Functions.h"
#include "Synth_API_Preface_Registers.h"
#include "Synth_API_Control_Registers.h"
#include "Synth_API_Status_Registers.h"
#include "Synth_API_Hibernation_Setup.h"


char new_line[] = "\n\r";

int main(void) {
	
	setupClock();
	initConsole();

	char user_input[MAX_INPUT_LENGTH], command[50], value[20];
	char out[20], ref[20];
	int outFactor, refFactor;
	unsigned short INT;
	unsigned long FRAC, MOD;

	while(1){

		clearArray(out, 20);
		clearArray(ref, 20);

//		clearArray(user_input, 100);
//		clearArray(command, 50);
//		clearArray(value, 20);
//
//		getString(user_input);
//		parseString(user_input, command, value);

		getString(out);
		printString(new_line);
		printString(new_line);
		getString(ref);

		double outFreq = ConvertStringToFrequency(out, &outFactor);
//		double outFreq = ConvertStringToFrequency(value, &outFactor);
		double refFreq = ConvertStringToFrequency(ref, &refFactor);

//		printString(new_line);
//		printString(new_line);
//		printFloat(outFreq);

		double result = GenerateFrequencyRatio(outFreq, outFactor, refFreq, refFactor);

		DetermineFeedbackValues(result, false, &INT, &FRAC, &MOD);

		printString(new_line);
		printString(new_line);
		printFloat(outFreq);
		printString(new_line);
		printString(new_line);
		printFloat(refFreq);
		printString(new_line);
		printString(new_line);
		printString("Enter: ");

	}


	return 0;
}





