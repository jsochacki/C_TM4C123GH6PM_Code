/*
  Author: Mason Edgar
  Supervisor: John Sochacki
  ViaSat Inc.
  Tempe, AZ

  8V97003 Frequency Synth API for the TM4C123GH6PM Project using the EK-TM4C123GXL Dev board

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


// Macros for selecting the chip-select polarity. The numbers here are arbitrary, the macro is for readability
#define CS_ACTIVE_HIGH 0
#define CS_ACTIVE_LOW  1

// Masks to isolate bytes in the 24-bit words
#define LOWER_8_BITS 0x000000FF
#define MIDDLE_8_BITS 0x0000FF00
#define UPPER_8_BITS 0x00FF0000

// Masks to isolate bytes in 16-bit words
#define TOP_HALF_16_BIT 0xFF00
#define BOTTOM_HALF_16_BIT 0x00FF

// Masks to isolate bytes in 32-bit words
#define TOP_8_32Bit 0xFF000000
#define UPPER_MIDDLE_32Bit 0x00FF0000
#define LOWER_MIDDLE_32Bit 0x0000FF00
#define BOTTOM_8_32Bit 0x000000FF


/* Macros Specific to Register Programming */


// These define the address for each register so that it can be combined with a data byte to form the full 24-bit frame
#define REG_10h 0x1000
#define REG_11h 0x1100
#define REG_12h 0x1200
#define REG_13h 0x1300
#define REG_14h 0x1400
#define REG_15h 0x1500
#define REG_16h 0x1600
#define REG_17h 0x1700
#define REG_18h 0x1800
#define REG_19h 0x1900
#define REG_1Ah 0x1A00
#define REG_1Bh 0x1B00
#define REG_1Ch 0x1C00
#define REG_1Dh 0x1D00
#define REG_1Eh 0x1E00
#define REG_1Fh 0x1F00
#define REG_20h 0x2000
#define REG_21h 0x2100
#define REG_22h 0x2200
#define REG_23h 0x2300
#define REG_24h 0x2400
#define REG_25h 0x2500

// Sets the Soft Reset pins in Register 0h. Further explained in the InitPrefaceRegister function comments
#define SOFT_RESET 0x81

// Sets the Read From Active Registers pin inside Register 1h. Further explained in the InitPrefaceRegister function comments
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
/*
 *                                      Preface Register
 *
 * By default, the SPI interface is in 3-wire mode with SDO in high-impedance, MSB-first mode,
 * and address is in auto-decrement mode. Setting the soft reset pins will ensure default operation
 *
 */
/*******************************************************************************************************/



void InitPrefaceRegister(void){

	sendWord_24Bit(SOFT_RESET); // Register 0h

	sendWord_24Bit(READ_FROM_ACTIVE_REGISTERS); // Register 1h

/*

             Register 0h

   Bits 3-0 must mirror bits 7-4
+--+ +--+ +--+ +--+ +--+ +--+ +--+ +--+
|  | |  | |  | |  | |  | |  | |  | |  |
|D7| |D6| |D5| |D4| |D3| |D2| |D1| |D0|
|  | |  | |  | |  | |  | |  | |  | |  |
+--+ +--+ +--+ +--+ +--+ +--+ +--+ +--+
 S    L    A    S     S    A    L    S
 O    S    D    D     D    D    S    O
 F    B    D    O     O    D    B    F
 T         R               R         T
      F    E    A     A    E    F
 R    I    S    C     C    S    I    R
 E    R    S    T     T    S    R    E
 S    S         I     I         S    S
 E    T    A    V     V    A    T    E
 T         S    E     E    S         T
           C               C
           E               E
           N               N
           D               D


___________________________________________________________________________________

Soft Reset Function:

0 = Normal operation
1 = Register reset. The device loads the default values into the registers 0002 -
00FF.
The content of the registers addresses 0000 an 0001 and the SPI engine are
not reset.
Soft reset bit D7 is mirrored with <Softreset> in bit position D0. Register reset
requires setting both SoftReset and <SoftReset> bits.

___________________________________________________________________________________

Least Significant Bit Position:

Defines the bit transmitted first in SPI transfers between slave and master.
0 = The most significant bit (D7) first
1 = The least significant bit (D0) first
LSBFirst bit D6 is mirrored with <LSBFirst> in bit position D1. Changing
LSBFirst to most significant bit requires setting both LSBFirst and <LSBFirst>
bits.

___________________________________________________________________________________

Address Ascend On:

0 = Address Ascend is off (Addresses auto-decrement in streaming SPI mode)
1 = Address Ascend is on (Addresses auto-increment in streaming SPI mode)
The AddressAscend bit specifies whether addresses are incremented or
decremented in streaming SPI transfers.
AddressAscend bit D5 is mirrored with <AddressAscend> in bit position D2.
Changing AddressAscend to �ON� requires to set both AddressAscend and
<AddressAscend> bits

___________________________________________________________________________________

SPI 3/4-Wire Mode:

Selects the unidirectional or bidirectional data transfer mode for the SDIO pin.

0 = SPI 3-wire mode:
- SDIO is the SPI bidirectional data I/O pin
- SDO pin is not used and is in high-impedance

1 = SPI 4-wire mode
- SDIO is the SPI data input pin
- SDO is the SPI data output pin

SDOActive bit D4 is mirrored with <SDOActive> in bit position D3. Changing
SDOActive to SPI 4-wire mode requires setting both SDOActive and
<SDOActive> bits.

___________________________________________________________________________________




            Register 1h

+--+ +--+ +--+ +--+ +--+ +--+ +--+ +--+
|  | |  | |  | |  | |  | |  | |  | |  |
|D7| |D6| |D5| |D4| |D3| |D2| |D1| |D0|
|  | |  | |  | |  | |  | |  | |  | |  |
+--+ +--+ +--+ +--+ +--+ +--+ +--+ +--+
 U    U    B    U    U    U    U    U
 N    N    U    N    N    N    N    N
 U    U    F    U    U    U    U    U
 S    S    F    S    S    S    S    S
 E    E    E    E    E    E    E    E
 D    D    R    D    D    D    D    D

           R
           E
           A
           D

           M
           O
           D
           E

___________________________________________________________________________________

Read Back Mode of the Buffer Registers:

0 = Read from active registers
1 = Read from the Buffer Register (case of Doubled Buffer Registers); If the
register being read is not doubled buffered, a 1 value will read from the active
register

___________________________________________________________________________________



*/

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
 */
/***************************************************************************************************************************************/



void SetFeedbackControlValues(unsigned short Nint, unsigned long Nfrac, unsigned long Nmod){

	/*

	 Since the chip has 8-bit registers, the values for Nint, Nfrac, and Nmod are spread out across
	 multiple (consecutive) registers. The code below breaks each argument down into single-byte
	 pieces and sends them to their appropriate register.

	*/


	// Integer Portion
	unsigned long Nint_top, Nint_bottom, Nint_bottom_word, Nint_top_word;

	SplitNumber_16Bit(Nint, &Nint_top, &Nint_bottom);

	Nint_bottom_word = Create24BitWord(Nint_bottom, REG_10h);
	Nint_top_word = Create24BitWord(Nint_top, REG_11h);

	sendWord_24Bit(Nint_bottom_word);
	sendWord_24Bit(Nint_top_word);

	// Fractional Portion
	unsigned long Nfrac_top, Nfrac_upper_middle, Nfrac_lower_middle, Nfrac_bottom;
	unsigned long Nfrac_top_word, Nfrac_uppermiddle_word, Nfrac_lowermiddle_word, Nfrac_bottom_word;

	SplitNumber_32Bit(Nfrac, &Nfrac_top, &Nfrac_upper_middle, &Nfrac_lower_middle, &Nfrac_bottom);

	Nfrac_bottom_word = Create24BitWord(Nfrac_bottom, REG_12h);
	Nfrac_lowermiddle_word = Create24BitWord(Nfrac_lower_middle, REG_13h);
	Nfrac_uppermiddle_word = Create24BitWord(Nfrac_upper_middle, REG_14h);
	Nfrac_top_word = Create24BitWord(Nfrac_top, REG_15h);

	sendWord_24Bit(Nfrac_bottom_word);
	sendWord_24Bit(Nfrac_lowermiddle_word);
	sendWord_24Bit(Nfrac_uppermiddle_word);
	sendWord_24Bit(Nfrac_top_word);

	// Modulus Portion
	unsigned long Nmod_top, Nmod_upper_middle, Nmod_lower_middle, Nmod_bottom;
	unsigned long Nmod_top_word, Nmod_uppermiddle_word, Nmod_lowermiddle_word, Nmod_bottom_word;

	SplitNumber_32Bit(Nmod, &Nmod_top, &Nmod_upper_middle, &Nmod_lower_middle, &Nmod_bottom);

	Nmod_bottom_word = Create24BitWord(Nmod_bottom, REG_16h);
	Nmod_lowermiddle_word = Create24BitWord(Nmod_lower_middle, REG_17h);
	Nmod_uppermiddle_word = Create24BitWord(Nmod_upper_middle, REG_18h);
	Nmod_top_word = Create24BitWord(Nmod_top, REG_19h);

	sendWord_24Bit(Nmod_bottom_word);
	sendWord_24Bit(Nmod_lowermiddle_word);
	sendWord_24Bit(Nmod_uppermiddle_word);
	sendWord_24Bit(Nmod_top_word);

}





























