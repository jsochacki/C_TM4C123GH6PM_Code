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
#include "Synth_API_Preface_Registers.h"
#include "Synth_API_Control_Registers.h"
#include "Synth_API_Status_Registers.h"


char new_line[] = "\n\r";


int main(void) {
	

	char user_input[MAX_INPUT_LENGTH], command[50], value[20];

	setupClock();
	InitConsole();
//	setupChipSelect(CS_ACTIVE_LOW);
//	setupSPI_8Bit();
//	InitPrefaceRegisters();

	while(1){

		clearArray(user_input, 100);
		clearArray(command, 50);
		clearArray(value, 20);

		getString(user_input);
		parseString(user_input, command, value);


		if(!strncmp(command, "setFrequency", 12)){
			printString(new_line);
			printString("Frequency set to ");
			printString(value);
			printString(new_line);
			printString("Enter command: ");
		}

		else if(!strncmp(command, "initDevice", 10)){
			printString("\n\r\n");
			printString("Initializing device...");
			printString("\n\r\n");
			printString("Enter command: ");
		}

		else{
			printString(new_line);
			printString("Command not recognized...");
			printString(new_line);
			printString("Enter command: ");
		}




	} // End while loop



	return 0;
}


