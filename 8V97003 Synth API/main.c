/*
  Author: Mason Edgar
  Supervisor: John Sochacki
  ViaSat Inc.
  Tempe, AZ

  8V97003 Frequency Synth API for the TM4C123GH6PM Project using the EK-TM4C123GXL dev board

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


#define CS_ACTIVE_HIGH 0
#define CS_ACTIVE_LOW  1
#define LOWER_8_BITS 0x000000FF
#define MIDDLE_8_BITS 0x0000FF00
#define UPPER_8_BITS 0x00FF0000
#define TOP_HALF_16_BIT 0xFF00
#define BOTTOM_HALF_16_BIT 0x00FF

// Macros Specific to Register Programming
#define SOFT_RESET 0x81
#define READ_FROM_ACTIVE_REGISTERS 0x100



// flag that makes sure chip select is asserted the correct way
int cs_config;



/*

+                                                                                                                                               +
+------------------------------------------------------+   24-BIT DATA WORD FORMAT   +----------------------------------------------------------+
+                                                                                                                                               +

+-----+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+
|     | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   |
| R/W | |A14| |A13| |A12| |A11| |A10| |A09| |A08| |A07| |A06| |A05| |A04| |A03| |A02| |A01| |A00| |D07| |D06| |D05| |D04| |D03| |D02| |D01| |D00|
|     | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   | |   |
+-----+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+
		|                                                                                       | |                                             |
		|                                                                                       | |                                             |
		|                            REGISTER LOCATION (15-BIT)                                 | |            REGISTER DATA (8-BIT)            |
		+                                                                                       + +                                             +


 */








/*******************************************************************************************************/
/*                                 Initial clock setup routine
/*******************************************************************************************************/

void setupClock(void){

	// Set the system clock to 40 MHz
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
}


/*******************************************************************************************************/
/*    Allows the user to decide whether the chip select pin uses Active Low or Active High protocol
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
/*             Creates the SPI interface in 8-bit mode for 24-bit word transmission
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
/*         Breaks down 24-bit word into 3 8-bit chunks and sends them over the SPI lines
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
/*
 *                                      Preface Register
 *
 * By default, the SPI interface is in 3-wire mode with SDO in high-impedance, MSB-first mode,
 * and address is in auto-decrement mode. Setting the soft reset pins will ensure default operation
 *
/*******************************************************************************************************/



void InitPrefaceRegister(void){

	sendWord_24Bit(SOFT_RESET); // Register 0h

	sendWord_24Bit(READ_FROM_ACTIVE_REGISTERS); // Register 1h

}



/***************************************************************************************************************************************/
/*                             Feedback Control Divider Registers
 *
 * The feedback divider N supports fractional division capability in the PLL feedback path. It consists
 * of an integer N divider of 16-bits, and a Fractional divider of 32-bits (FRAC) over 32-bits (MOD).
 * The 16 INT bits (Nint[15:0] in the Feedback Divider Control Registers) set the integer part of the
 * feedback division ratio. The 32 FRAC bits (Bit Nfrac[31:0] in the Feedback Divider Control Registers)
 * set the numerator of the fraction that goes into the Sigma Delta modulator. The 32 MOD bits
 * (Nmod[31:0] in the Feedback Divider Control Registers) set the denominator of the fraction that goes
 * into the Sigma Delta modulator.
 *
 *
 * FB Divider Integer Portion: Minimum divide ratio is 7d. Default is 31d
 *
 * FB Divider Fractional Portion: Nfrac is the numerator value of the fractional divide ratio. It is programmable from 0 to (Nmod - 1).
 *
 * FB divider Modulus Portion: Range is 2d - 4,294,967,295d. Default is 4,294,966,784d
 *
/***************************************************************************************************************************************/



void SetFeedbackControlValues(unsigned short Nint, unsigned long Nfrac, unsigned long Nmod){

		unsigned short temp_top = Nint & TOP_HALF_16_BIT;
		unsigned short temp_bottom = Nint & BOTTOM_HALF_16_BIT;

		unsigned long Nint_top_half = temp_top >> 8;
		unsigned long Nint_bottom_half = temp_bottom;

		Nint_bottom_half = Nint_bottom_half | 0x10; // Register 10h
		Nint_top_half = Nint_top_half | 0x11; // Register 11h

		sendWord_24Bit(Nint_bottom_half);
		sendWord_24Bit(Nint_top_half);








}





























