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
	setupSPI_8Bit();

	char out[20], ref[20];
	int outFactor, refFactor;
	unsigned short nINT;
	unsigned long nFRAC, nMOD;

	while(1){

		clearArray(out, 20);
		clearArray(ref, 20);

		getString(out);
		printString(new_line);
		printString(new_line);
		getString(ref);

		double outFreq = ConvertStringToFrequency(out, &outFactor);

		double refFreq = ConvertStringToFrequency(ref, &refFactor);


		double result = GenerateFrequencyRatio(outFreq, outFactor, refFreq, refFactor);

		DetermineFeedbackValues(result, false, &nINT, &nFRAC, &nMOD);

		printString(new_line);
		printString(new_line);
		printFloat(outFreq);
		printString(new_line);
		printString(new_line);
		printFloat(refFreq);
		printString(new_line);
		printString(new_line);
		printString("Enter: ");

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//		char ref[20], out[20], intMode[5];
//		int out_factor = 0, intModeDecision = 0, ref_factor = 0;
//		double output_freq = 0, result = 0, input_freq = 0;
//		unsigned short nINT;
//		unsigned long nFRAC, nMOD;
//
//		clearArray(out, 20);
//		clearArray(ref, 20);
//		clearArray(intMode, 5);
//
//		printString(new_line);
//		printString(new_line);
//		printString("Enter the desired output frequency: ");
//		getString(out);
//
//
//		printString(new_line);
//		printString(new_line);
//		printString("Enter the desired input frequency: ");
//		getString(ref);
//
//
//		output_freq = ConvertStringToFrequency(out, &out_factor);
//
//		input_freq = ConvertStringToFrequency(ref, &ref_factor);
//
//		result = GenerateFrequencyRatio(output_freq, out_factor, input_freq, ref_factor);
//
//
//		printString(new_line);
//		printString(new_line);
//		printString("Integer mode? (\"y\" for integer, \"n\" for fractional): ");
//		getString(intMode);
//
//
//		intModeDecision = ConvertStringToBool(intMode);
//
//
//
////		result = GenerateFrequencyRatio(output_freq, out_factor, InternalReferenceValue, internalRefFactor);
//		DetermineFeedbackValues(result, intModeDecision, &nINT, &nFRAC, &nMOD);
//		SetFeedbackControlValues(nINT, nFRAC, nMOD);
//		printString(new_line);
//		printString(new_line);
//		printString("Frequency has been set.");
//		printString(new_line);
//		printString(new_line);
//		printString("Enter command: ");

	}


	return 0;
}





