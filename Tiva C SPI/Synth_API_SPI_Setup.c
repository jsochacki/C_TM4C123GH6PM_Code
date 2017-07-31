
/*
 * Synth_API_SPI_Setup.c
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
#include "Synth_API_SPI_Setup.h"


int cs_config;


// Fixed-point arithmetic

#define SHIFT_AMOUNT 16 // 2^16 = 65536

const int fractionMask = 0xFFFFFFFFFFFFFFFF >> (64 - SHIFT_AMOUNT);

#define DoubleToFixed(x) ((x) * (double)((uint64_t)1 << SHIFT_AMOUNT))
#define FixedToDouble(x) ((double)(x) / (double)((uint64_t)1 << SHIFT_AMOUNT))
#define FractionPart(x) ((x) & fractionMask)
#define FixedMultiply(x,y) (((x) * (y)) >> SHIFT_AMOUNT)
#define FixedDivide(x,y) (((x) << SHIFT_AMOUNT) / (y))


/*******************************************************************************************************/
/*                                 Initial clock setup routine                                         */
/*******************************************************************************************************/

void setupClock(void){

	// Set the system clock to 40 MHz
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
}




/*******************************************************************************************************/
/*    Allows the user to decide whether the chip select pin uses Active Low or Active High protocol    */
/*******************************************************************************************************/

void setupChipSelect(int mode){

	// Pin B1 is the new chip select
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);

	// Allow user to decide if the chip select is active high or active low
	if(mode == CS_ACTIVE_HIGH){

		MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00);
		cs_config = 1;
	}

	else if(mode == CS_ACTIVE_LOW){

		MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x2);
		cs_config = 0;
	}
}


/*******************************************************************************************************/
/*             Creates the SPI interface in 8-bit mode for 24-bit word transmission                    */
/*******************************************************************************************************/

void setupSPI_8Bit(void){

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);  // Enable SPI Port 0

	// Configure the GPIO pins to access SPI port 0
	MAP_GPIOPinConfigure(GPIO_PA4_SSI0RX);
	MAP_GPIOPinConfigure(GPIO_PA5_SSI0TX);
	MAP_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
	MAP_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
	MAP_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

	// Begin configuration of the SPI0 interface
	MAP_SSIDisable(SSI0_BASE);

	// Sets the clock polarity to steady state low and rising edge
	MAP_SSIConfigSetExpClk(SSI0_BASE, MAP_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 500000, 8);

	MAP_SSIEnable(SSI0_BASE);

}



/*******************************************************************************************************/
/*            Splits 16-bit number into 2 8-bit data chunks for register programming                   */
/*******************************************************************************************************/

void SplitNumber_16Bit(unsigned short original, unsigned long* top_half, unsigned long* bottom_half) {

	unsigned short top = original & TOP_HALF_16_BIT;
	unsigned short bottom = original & BOTTOM_HALF_16_BIT;

	unsigned long top_32bit = top >> 8;
	unsigned long bottom_32bit = bottom;

	*top_half = top_32bit;
	*bottom_half = bottom_32bit;
}



/*******************************************************************************************************/
/*            Splits 32-bit number into 4 8-bit data chunks for register programming                   */
/*******************************************************************************************************/

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


/*******************************************************************************************************/
/*     Determines the MOD and FRAC values by minimizing the MOD value (for use by Feedback Control)    */
/*******************************************************************************************************/

void FixedPointMinimizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD) {

	double modulus_min = 2;

	uint64_t fixed_mod_min = DoubleToFixed(modulus_min);
	uint64_t fixed_ratio = DoubleToFixed(ratio);
	uint64_t fixed_frac = FixedMultiply(fixed_ratio, fixed_mod_min);

	double real_frac = FixedToDouble(fixed_frac);

	while ((real_frac - (unsigned long)real_frac) != 0) {
		fixed_mod_min = fixed_mod_min + 1;
		fixed_frac = FixedMultiply(fixed_ratio, fixed_mod_min);
		real_frac = FixedToDouble(fixed_frac);
	}

	double real_mod_min = FixedToDouble(fixed_mod_min);

	*nFRAC = (unsigned long)real_frac;
	*nMOD = (unsigned long)real_mod_min;

}


/*******************************************************************************************************/
/*     Determines the MOD and FRAC values by maximizing the MOD value (for use by Feedback Control)    */
/*******************************************************************************************************/

void FixedPointMaximizeMOD(double ratio, unsigned long* nFRAC, unsigned long* nMOD) {

	double modulus_max = 4294967295;

	uint64_t fixed_mod_max = DoubleToFixed(modulus_max);
	uint64_t fixed_ratio = DoubleToFixed(ratio);
	uint64_t fixed_frac = FixedMultiply(fixed_ratio, fixed_mod_max);

	double real_frac = FixedToDouble(fixed_frac);

	while ((real_frac - (unsigned long)real_frac) != 0) {
		fixed_mod_max = fixed_mod_max - 1;
		fixed_frac = FixedMultiply(fixed_ratio, fixed_mod_max);
		real_frac = FixedToDouble(fixed_frac);
	}

	double real_mod_max = FixedToDouble(fixed_mod_max);

	*nFRAC = (unsigned long)real_frac;
	*nMOD = (unsigned long)real_mod_max;
}


/****************************************************************************************************************************************/
/*                           Converts desired frequency & reference frequency into programmable values                                  */
/****************************************************************************************************************************************/

void DetermineFeedbackValues(double freq_ratio, bool integer_mode, unsigned short* nINT, unsigned long* nFRAC, unsigned long* nMOD){

	unsigned long frac, mod;

	*nINT = (unsigned short)freq_ratio;

	if(integer_mode){

		*nFRAC = 0;
		*nMOD = 2;
	}

	else{

		double decimal_portion = freq_ratio - (unsigned short)freq_ratio;

		FixedPointMaximizeMOD(decimal_portion, &frac, &mod); // Choose whether you want to maximize the modulus or minimize it

	//	FixedPointMinimizeMOD(decimal_portion, &frac, &mod);

		*nFRAC = frac;
		*nMOD = mod;
	}


}


/*******************************************************************************************************/
/*            Creates properly formatted data word to program a particular register                    */
/*******************************************************************************************************/

unsigned long Create24BitWord(unsigned long data, unsigned long reg){

	unsigned long word;

	word = data | reg;

	return word;

}


/*******************************************************************************************************/
/*         Breaks down 24-bit word into 3 8-bit chunks and sends them over the SPI lines               */
/*******************************************************************************************************/

void sendWord_24Bit(unsigned long word){

	// Break 24-bit word into 3 8-bit words
	uint32_t upper, middle, lower;
	unsigned long temp1, temp2, temp3;

	temp1 = word & UPPER_8_BITS;
	temp2 = word & MIDDLE_8_BITS;
	temp3 = word & LOWER_8_BITS;

	upper = temp1 >> 16;
	middle = temp2 >> 8;
	lower = temp3;

	// Enable chip select to start transfer
	if(cs_config == 1){ MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x2); }
	else if(cs_config == 0) { MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00); }

	// Send all 3 pieces of the original word, MSB first
	MAP_SSIDataPut(SSI0_BASE, upper);
	MAP_SSIDataPut(SSI0_BASE, middle);
	MAP_SSIDataPut(SSI0_BASE, lower);

	while(MAP_SSIBusy(SSI0_BASE)){};

	// Close the frame by de-asserting chip select
	if(cs_config == 1){ MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00); }
	else if(cs_config == 0) { MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x2); }

}

/*******************************************************************************************************/
/*                 Reads back an 8-bit value from a given register address                             */
/*******************************************************************************************************/


uint32_t GetRegisterValue(unsigned long address){

	uint32_t regData;
	int i;

	for(i=0; i<2; i++){ regData = readWord_24Bit(address); }

	return regData;
}


/*********************************************************************************************************/
/*          Subsidiary function for GetRegisterValue that actually handles the SPI transaction           */
/*********************************************************************************************************/

uint32_t readWord_24Bit(unsigned long word){

	// Break 24-bit word into 3 8-bit words
	uint32_t upper, middle, lower;
	unsigned long temp1, temp2, temp3;
	uint32_t dummyVar, returnValue;

	while(MAP_SSIDataGetNonBlocking(SSI0_BASE, &dummyVar)){ }; // Clear out unwanted bits from Rx FIFO

	temp1 = word & UPPER_8_BITS;
	temp2 = word & MIDDLE_8_BITS;
	temp3 = word & LOWER_8_BITS;

	upper = temp1 >> 16;
	middle = temp2 >> 8;
	lower = temp3;

	// Enable chip select to start transfer
	if(cs_config == 1){ MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x2); }
	else if(cs_config == 0) { MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00); }

	// Send all 3 pieces of the original word, MSB first
	MAP_SSIDataPut(SSI0_BASE, upper);
	MAP_SSIDataPut(SSI0_BASE, middle);
	MAP_SSIDataPut(SSI0_BASE, lower);

	while(MAP_SSIBusy(SSI0_BASE)){};

	// Close the frame by de-asserting chip select
	if(cs_config == 1){ MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00); }
	else if(cs_config == 0) { MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x2); }

	MAP_SSIDataGet(SSI0_BASE, &returnValue);


	return returnValue;

}
