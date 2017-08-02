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
#include "Synth_API_Hibernation_Setup.h"


char new_line[] = "\n\r";


int main(void) {
	

	char user_input[MAX_INPUT_LENGTH], command[50], value[20];

	setupClock();
	initConsole();
	initHibernationModule();
	setupChipSelect(CS_ACTIVE_LOW);
	setupSPI_8Bit();
	initPrefaceRegisters();


	while(1){

		clearArray(user_input, 100);
		clearArray(command, 50);
		clearArray(value, 20);

		getString(user_input);
		parseString(user_input, command, value);


		if(!strncmp(command, "setFrequency", 12)){

			char ref[20], intMode[3];
			int out_factor, ref_factor, intModeDecision;
			double output_freq, reference_freq, result;
			unsigned short nINT;
			unsigned long nFRAC, nMOD;

			clearArray(ref, 20);
			clearArray(intMode, 3);
			printString(new_line);
			printString(new_line);
			printString("Integer mode? (\"y\" for integer, \"n\" for fractional): ");
			getString(intMode);
			printString(new_line);
			printString(new_line);
			printString("Enter the reference frequency: ");
			getString(ref);
			intModeDecision = ConvertStringToBool(intMode);
			reference_freq = ConvertStringToFrequency(ref, &ref_factor);
			output_freq = ConvertStringToFrequency(value, &out_factor);
			result = GenerateFrequencyRatio(output_freq, out_factor, reference_freq, ref_factor);
			DetermineFeedbackValues(result, intModeDecision, &nINT, &nFRAC, &nMOD);
			SetFeedbackControlValues(nINT, nFRAC, nMOD);
			printString(new_line);
			printString(new_line);
			printString("Frequency set to ");
			printString(value);
			printString(new_line);
			printString(new_line);
			printString("Enter command: ");
		}

		else if(!strncmp(command, "setOutputPower", 14)){

			char RF_OUTB[3], outDoubler[3], outDivider[5], MuteUntilLD[3], QA_pwr[10], QB_pwr[10];
			int RF_OUTB_Result, outDoublerResult, MuteUntilLDResult;
			unsigned long QA_pwrResult, QB_pwrResult = 0, outDividerResult;

			printString(new_line);
			printString(new_line);
			printString("Available power settings: -2dBm to +12dBm in steps of 2dBm");
			printString(new_line);
			printString(new_line);
			printString("Power setting for RF_OUTA (Ex. \"+2dBm\"): ");
			getString(QA_pwr);
			QA_pwrResult = ConvertStringToPowerSetting(QA_pwr);
			printString(new_line);
			printString(new_line);
			printString("Do you want to also use the auxiliary RF_OUTB? (y/n): ");
			getString(RF_OUTB);
			printString(new_line);
			printString(new_line);
			RF_OUTB_Result = ConvertStringToBool(RF_OUTB);

			if(RF_OUTB_Result){
				printString("Power setting for RF_OUTB (Ex. \"+2dBm\"): ");
				getString(QB_pwr);
				QB_pwrResult = ConvertStringToPowerSetting(QB_pwr);
				printString(new_line);
				printString(new_line);
			}

			printString("Enable the output doubler? (y/n): ");
			getString(outDoubler);
			printString(new_line);
			printString(new_line);
			printString("Set the output divider value (1, 2, 4, 8, 16, 32): ");
			getString(outDivider);
			printString(new_line);
			printString(new_line);
			outDoublerResult = ConvertStringToBool(outDoubler);
			outDividerResult = ConvertStringToNumber(outDivider);
			printString("Mute outputs until lock has been detected? (y/n): ");
			getString(MuteUntilLD);
			MuteUntilLDResult = ConvertStringToBool(MuteUntilLD);
			SetupOutputControl(true, QA_pwrResult, RF_OUTB_Result, QB_pwrResult, MuteUntilLDResult, outDoublerResult, outDividerResult);
			printString(new_line);
			printString(new_line);
			printString("Enter command: ");
		}

		else if(!strncmp(command, "lockStatus", 10)){

			ReadFromStatusRegisters("DigLock");
			printString("Enter command: ");
		}

		else if(!strncmp(command, "sleep", 5)){

			printString(new_line);
			printString(new_line);
			printString("Going to sleep...");
			printString(new_line);
			printString(new_line);
			MAP_SysCtlDelay(100000);
			MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00);
			MAP_HibernateRequest();
			while(1){ };
		}

		else{

			printString(new_line);
			printString(new_line);
			printString("Command not recognized...");
			printString(new_line);
			printString(new_line);
			printString("Enter command: ");
		}




	} // End while loop



	return 0;
}


