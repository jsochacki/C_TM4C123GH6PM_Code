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


/*

 These define the address for each register so that it can be combined with a data byte to form the full 24-bit frame.
  - All of the below register macros include the '0' MSB which indicates a write operation.

*/
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
#define REG_26h 0x2600
#define REG_27h 0x2700
#define REG_28h 0x2800
#define REG_29h 0x2900
#define REG_2Ah 0x2A00
#define REG_2Bh 0x2B00
#define REG_2Ch 0x2C00
#define REG_2Dh 0x2D00
#define REG_2Eh 0x2E00
#define REG_2Fh 0x2F00
#define REG_30h 0x3000
#define REG_31h 0x3100
#define REG_32h 0x3200
#define REG_33h 0x3300
#define REG_34h 0x3400
#define REG_35h 0x3500
#define REG_36h 0x3600
#define REG_37h 0x3700
#define REG_38h 0x3800
#define REG_39h 0x3900
#define REG_3Ah 0x3A00
#define REG_3Bh 0x3B00
#define REG_3Ch 0x3C00
#define REG_3Dh 0x3D00
#define REG_3Eh 0x3E00
#define REG_3Fh 0x3F00
#define REG_40h 0x4000
#define REG_41h 0x4100
#define REG_42h 0x4200
#define REG_43h 0x4300
#define REG_44h 0x4400
#define REG_45h 0x4500
#define REG_46h 0x4600
#define REG_47h 0x4700
#define REG_48h 0x4800
#define REG_49h 0x4900


// Sets the Soft Reset pins in Register 0h. Further explained in the InitPrefaceRegister function comments
#define SOFT_RESET 0x81

// Sets the Read From Active Registers pin inside Register 1h. Further explained in the InitPrefaceRegister function comments
#define READ_FROM_ACTIVE_REGISTERS 0x100

// Charge Pump Register: High Impedance Macro
#define CP_HIGH_Z 0x00000020

// Output Control Register Macros
#define QA_ENABLE_MUTE_UNTIL_LOCK_DETECT 0x38
#define QA_ENABLE_NO_MUTE 0x18
#define QB_ENABLE 0x18
#define MUTE_UNTIL_LOCK_DETECT_WITHOUT_QA_ENABLED 0x28
#define DISABLE_MUTE_WITHOUT_QA_ENABLED 0x08
#define OutputDoublerEnable 0x80

// Lock Detection Control Register Macros
#define UNLOCK_DETECTION 0x08
#define NO_UNLOCK_DETECTION 0x00



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


                               Feedback Control Register Map (ADDR is in hex)

  +------------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+
  |            |           |           |           |           |           |           |           |           |
  |    ADDR    |    D7     |    D6     |    D5     |    D4     |    D3     |    D2     |    D1     |    D0     |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0010    |  Nint<7>  |  Nint<6>  |  Nint<5>  |  Nint<4>  |  Nint<3>  |  Nint<2>  |  Nint<1>  |  Nint<0>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0011    |  Nint<15> |  Nint<14> |  Nint<13> |  Nint<12> |  Nint<11> |  Nint<10> |  Nint<9>  |  Nint<8>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0012    |  Nfrac<7> |  Nfrac<6> |  Nfrac<5> |  Nfrac<4> |  Nfrac<3> |  Nfrac<2> |  Nfrac<1> |  Nfrac<0> |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0013    | Nfrac<15> | Nfrac<14> | Nfrac<13> | Nfrac<12> | Nfrac<11> | Nfrac<10> | Nfrac<9>  | Nfrac<8>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0014    | Nfrac<23> | Nfrac<22> | Nfrac<21> | Nfrac<20> | Nfrac<19> | Nfrac<18> | Nfrac<17> | Nfrac<16> |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0015    | Nfrac<31> | Nfrac<30> | Nfrac<29> | Nfrac<28> | Nfrac<27> | Nfrac<26> | Nfrac<25> | Nfrac<24> |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0016    |  Nmod<7>  |  Nmod<6>  |  Nmod<5>  |  Nmod<4>  |  Nmod<3>  |  Nmod<2>  |  Nmod<1>  |  Nmod<0>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0017    |  Nmod<15> |  Nmod<14> |  Nmod<13> |  Nmod<12> |  Nmod<11> |  Nmod<10> |  Nmod<9>  |  Nmod<8>  |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0018    |  Nmod<23> |  Nmod<22> |  Nmod<21> |  Nmod<20> |  Nmod<19> |  Nmod<18> |  Nmod<17> |  Nmod<16> |
  |            |           |           |           |           |           |           |           |           |
  +------------------------------------------------------------------------------------------------------------+
  |            |           |           |           |           |           |           |           |           |
  |    0019    |  Nmod<31> |  Nmod<30> |  Nmod<29> |  Nmod<28> |  Nmod<27> |  Nmod<26> |  Nmod<25> |  Nmod<24> |
  |            |           |           |           |           |           |           |           |           |
  +------------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+-----------+

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



/*******************************************************************************************************/
/*
 *                               Band Select Clock Divider Registers
 *
 *   Band Select Clock Divider:  Not Allowed: 0d
 *                               Default: 2,560d
 *                               Range: 1d - 131,071d
 *
 *   This ratio should be set so that F_pfd / BndSelDiv is < 100kHz.
 */
/*******************************************************************************************************/

void SetBandSelectClockDivider(unsigned short divider_value){


/*

                                                    Band Select Clock Diver Control Register Map

  +------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
  |      |                |                |                |                |                |                |                |                |
  | ADDR |       D7       |       D6       |       D5       |       D4       |       D3       |       D2       |       D1       |       D0       |
  |      |                |                |                |                |                |                |                |                |
  +----------------------------------------------------------------------------------------------------------------------------------------------+
  |      |                |                |                |                |                |                |                |                |
  | 0022 | BndSelDiv <07> | BndSelDiv <06> | BndSelDiv <05> | BndSelDiv <04> | BndSelDiv <03> | BndSelDiv <02> | BndSelDiv <01> | BndSelDiv <00> |
  |      |                |                |                |                |                |                |                |                |
  +----------------------------------------------------------------------------------------------------------------------------------------------+
  |      |                |                |                |                |                |                |                |                |
  | 0023 |     Unused     |     Unused     |     Unused     | BndSelDiv <12> | BndSelDiv <11> | BndSelDiv <10> | BndSelDiv <09> | BndSelDiv <08> |
  |      |                |                |                |                |                |                |                |                |
  +------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+



*/

	unsigned long upper, lower, upper_word, lower_word;

	SplitNumber_16Bit(divider_value, &upper, &lower);

	lower_word = Create24BitWord(lower, REG_22h);
	upper_word = Create24BitWord(upper, REG_23h);


	sendWord_24Bit(lower_word);
	sendWord_24Bit(upper_word);

}




void SetupChargePump(unsigned long icp_pmos, unsigned long icp_nmos, bool CP_HiZ, unsigned long icp_bleeder){


/*

                                                             Charge Pump Control Register Map

  +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
  |                |                |                |                |                |                |                |                |                |
  |      ADDR      |       D7       |       D6       |       D5       |       D4       |       D3       |       D2       |       D1       |       D0       |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002D      |     UNUSED     |     UNUSED     |   Icp_pmos<5>  |   Icp_pmos<4>  |   Icp_pmos<3>  |   Icp_pmos<2>  |   Icp_pmos<1>  |   Icp_pmos<0>  |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002E      |     UNUSED     |     UNUSED     |   Icp_nmos<5>  |   Icp_nmos<4>  |   Icp_nmos<3>  |   Icp_nmos<2>  |   Icp_nmos<1>  |   Icp_nmos<0>  |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002F      |     UNUSED     |     UNUSED     |     CP_HiZ     | Icp_bleeder<4> | Icp_bleeder<3> | Icp_bleeder<2> | Icp_bleeder<1> | Icp_bleeder<0> |
  |                |                |                |                |                |                |                |                |                |
  +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


 In order to put the charge pump into three-state mode, the user must set the bits CP_HiZ (Bit D5) to 1 in Register 47d in the Charge Pump Control Registers.

 This bit should be set to 0 for normal operation.

________________________________________________

 Charge Pump Pmos Current Setting:

 00 0000 = 166 uA

 00 0001 = 333 uA

 ...

 01 1101 = 5 mA (default)

 ...

 11 1111 = 10.66 mA

________________________________________________

 Charge Pump Nmos Current Setting

 00 0000 = 166 uA

 00 0001 = 333 uA

 ...

 01 1101 = 5 mA (default)

 ...

 01 1111 = 10.66 mA

________________________________________________

 Charge Pump High-Impedance Control

 0 = Charge Pump Active

 1 = Charge Pump High-Impedance



 Charge Pump High Impedance Macros:

 CP_HIGH_Z 0x00000020

________________________________________________

 Charge Pump Bleeder Current Setting

 0 0000 = Off (0 uA) (Default)

 0 0001 = 5.3 uA

 0 0010 = 10.6 uA

 ...

 1 1111 = 166 uA

________________________________________________

*/


	unsigned long icp_pmos_word, icp_nmos_word, icp_bleeder_word, bleeder_with_HiZ;

	icp_pmos_word = Create24BitWord(icp_pmos, REG_2Dh);
	icp_nmos_word = Create24BitWord(icp_nmos, REG_2Eh);


	if(CP_HiZ){

		bleeder_with_HiZ = icp_bleeder | CP_HIGH_Z;

		icp_bleeder_word = Create24BitWord(bleeder_with_HiZ, REG_2Fh);
	}

	else{

		icp_bleeder_word = Create24BitWord(icp_bleeder, REG_2Fh);
	}


	sendWord_24Bit(icp_pmos_word);
	sendWord_24Bit(icp_nmos_word);
	sendWord_24Bit(icp_bleeder_word);

}



void SetupOutputControl(bool QA_enable, unsigned long QA_power, bool QB_enable, unsigned long QB_power, bool MuteUntil_LockDetect, bool OutDoublerEnable, unsigned long OutDivider){

/*

  The 8V97003 device provides two differential outputs. These two outputs generate the same frequency equal to f_VCO when bypassing the optional output doubler
  and the optional output divider M0, or to 2 � f_VCO (up to 18GHz) when using the optional output doubler, or an integer division of the VCO frequency f_VCO.
  The division ratios of the output divider are provided in the Outputs Control Registers (see below).

  Each output buffer RF_OUTA and RF_OUTB offer a programmable RF output power. The programmable output power settings can be selected from -2dBm to +12dBm with steps of 2dBm.
  The RF output power can be programmed via the bits QA_pwr[7:0] and QB_pwr[7:0].

  The 8V97003 offers an auxiliary output (RF_OUTB). If the auxiliary output stage is not used, it can be powered down by using the QB_Ena bit in the Outputs Control Registers
  (refer to below register map). The outputs can be disabled until the part achieves lock. To enable this mode, the user will set the Mute_until_LD bit in the Outputs Control
  Registers (refer to below register map). The MUTE pin can be used to mute all outputs and be used as a similar function.


                                                               Output Control Register Map

  +---------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
  |               |                |                |                |                |                |                |                |                |
  |     ADDR      |       D7       |       D6       |       D5       |       D4       |       D3       |       D2       |       D1       |       D0       |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0033      |    QA_pwr<7>   |    QA_pwr<6>   |    QA_pwr<5>   |    QA_pwr<4>   |    QA_pwr<3>   |    QA_pwr<2>   |    QA_pwr<1>   |    QA_pwr<0>   |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0034      |       0        |     UNUSED     | Mute_Until_LD  |     QA_ena     |       1        |       0        |       0        |       0        |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0035      |   QB_pwr<7>    |    QB_pwr<6>   |    QB_pwr<5>   |    QB_pwr<4>   |    QB_pwr<3>   |    QB_pwr<2>   |    QB_pwr<1>   |    QB_pwr<0>   |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0036      |       0        |     UNUSED     |     UNUSED     |     QB_ena     |       1        |       0        |       0        |       0        |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0037      |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0038      |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     0039      |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     003A      |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |    RESERVED    |
  |               |                |                |                |                |                |                |                |                |
  +-------------------------------------------------------------------------------------------------------------------------------------------------------+
  |               |                |                |                |                |                |                |                |                |
  |     003B      | OutDoubler_Ena |     UNUSED     |     UNUSED     |     UNUSED     |     UNUSED     |   OutDiv <2>   |   OutDiv <1>   |   OutDiv <0>   |
  |               |                |                |                |                |                |                |                |                |
  +---------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


__________________________________________________________________________________

 QA_Power:

 RF_OUTA Power Setting

 0000 0000 = OFF (Default)
 0000 0001 = -2dBm
 0000 0011 = 0dBm
 0000 0111 = +2dBm
 0000 1111 = +4dBm
 0001 1111 = +6dBm
 0011 1111 = +8dBm
 0111 1111 = +10dBm
 1111 1111 = +12dBm

__________________________________________________________________________________

 QA_ena:

 RF_OUTA Enable

 0 = RF_OUTA is OFF (Default)
 1 = RF_OUTA is Enabled

__________________________________________________________________________________

 QB_Power:

 RF_OUTB Power Setting

 0000 0000 = OFF (Default)
 0000 0001 = -2dBm
 0000 0011 = 0dBm
 0000 0111 = +2dBm
 0000 1111 = +4dBm
 0001 1111 = +6dBm
 0011 1111 = +8dBm
 0111 1111 = +10dBm
 1111 1111 = +12dBm

__________________________________________________________________________________

 QB_ena:

 RF_OUTB Enable

 0 = RF_OUTB is OFF (Default)
 1 = RF_OUTB is Enabled

__________________________________________________________________________________

 Mute_Until_LD:

 Mute until Lock Detect selection

 0: Outputs are enabled independent of Lock Detect (Default)
 1: Outputs are enabled only when Lock Detect is high

__________________________________________________________________________________

 Out_Doubler_Ena:

 RF Output Doubler Enable

 0 = RF Output Doubler Disabled (default)
 1 = RF Output Doubler Enabled

 OutDoubler_Ena may only be set to 1 if the VCO frequency is not greater than 9GHz.

__________________________________________________________________________________

 OutDivider:

 RF Output Divider Settings

 000 = Divide By 1
 001 = Divide By 2
 010 = Divide By 4
 011 = Divide By 8
 100 = Divide By 16
 101 = Divide By 32
 110 = Unused
 111 = Unused

__________________________________________________________________________________


*/


	if(QA_enable){

		unsigned long QA_power_word, QA_enable_word;

		QA_power_word = Create24BitWord(QA_power, REG_33h);

		sendWord_24Bit(QA_power_word);

		if(MuteUntil_LockDetect){ QA_enable_word = Create24BitWord(QA_ENABLE_MUTE_UNTIL_LOCK_DETECT, REG_34h); }

		else{ QA_enable_word = Create24BitWord(QA_ENABLE_NO_MUTE, REG_34h); }

		sendWord_24Bit(QA_enable_word);

	}

	if(QB_enable){

		unsigned long QB_power_word, QB_enable_word, Enable_MuteUntilLockDetect_WithoutQA_word, Disable_MuteUntilLockDetect_WithoutQA_word;

		QB_power_word = Create24BitWord(QB_power, REG_35h);

		sendWord_24Bit(QB_power_word);

		if(QA_enable){

			// QA already set the MuteUntilLockDetect bit to the appropriate value, so we only need to enable the QB_enable bit
			QB_enable_word = Create24BitWord(QB_ENABLE, REG_36h);
			sendWord_24Bit(QB_enable_word);

		}

		else if(MuteUntil_LockDetect){

			// Set the MuteUntilLockDetect Bit inside register 34h since QA didn't set it already
			Enable_MuteUntilLockDetect_WithoutQA_word = Create24BitWord(MUTE_UNTIL_LOCK_DETECT_WITHOUT_QA_ENABLED, REG_34h);
			sendWord_24Bit(Enable_MuteUntilLockDetect_WithoutQA_word);

			// Set the QB_enable pin
			QB_enable_word = Create24BitWord(QB_ENABLE, REG_36h);
			sendWord_24Bit(QB_enable_word);

		}

		else if(!MuteUntil_LockDetect){

			// Clear the MuteUntilLockDetect Bit inside register 34h since QA didn't do it already
			Disable_MuteUntilLockDetect_WithoutQA_word = Create24BitWord(DISABLE_MUTE_WITHOUT_QA_ENABLED, REG_34h);
			sendWord_24Bit(Disable_MuteUntilLockDetect_WithoutQA_word);

			// Set the QB_enable pin
			QB_enable_word = Create24BitWord(QB_ENABLE, REG_36h);
			sendWord_24Bit(QB_enable_word);

		}

	}

	unsigned long output_divider_word, output_divider_value;

	if(OutDoublerEnable){

		output_divider_value = OutDivider | OutputDoublerEnable;
		output_divider_word = Create24BitWord(output_divider_value, REG_3Bh);
		sendWord_24Bit(output_divider_word);
	}

	else{
		output_divider_word = Create24BitWord(OutDivider, REG_3Bh);
		sendWord_24Bit(output_divider_word);
	}



}




void SetupLockDetect(bool Unlock_Detection, unsigned long LD_PinMode, unsigned long LD_Precision){

/*


                                                                        Lock Detect Control Register Map

  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       ADDR       |        D7        |        D6        |        D5        |        D4        |        D3        |        D2        |        D1        |        D0        |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       0026       |      UNUSED      |      UNUSED      |      UNUSED      |      UNUSED      | LDUnlockDetectEn |        0         |        0         |       0          |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       0027       |      UNUSED      |      UNUSED      |   LDPinMode<1>   |   LDPinMode<0>   |      UNUSED      |      LPD<2>      |      LPD<1>      |      LPD<0>      |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+


________________________________________________________________________

 Lock Detect - Unlock Detection

 0 = Disable (default)
 1 = Enable (a recalibration occurs if an unlock on LD is detected)

________________________________________________________________________

 LD Pin Mode

 00 = Digital Lock Detect (default); Normal lock detector function
 01 = Calibration done
 10 = Low
 11 = High

________________________________________________________________________

 Lock Detector Precision setting (ns)

 000 = 0.5 (default)
 001 = 1.0
 010 = 1.8
 011 = 3.0
 100 = 6.4
 101 = 6.4
 110 = 10.4
 111 = 10.4

 These values are under review because they don't really make sense rn
________________________________________________________________________


*/

	unsigned long unlock_detection_word, LD_PinMode_and_Precision, LD_PinMode_and_Precision_word;

	if(Unlock_Detection){

		unlock_detection_word = Create24BitWord(UNLOCK_DETECTION, REG_26h);
		sendWord_24Bit(unlock_detection_word);
	}

	else if(!Unlock_Detection){

		unlock_detection_word = Create24BitWord(NO_UNLOCK_DETECTION, REG_26h);
		sendWord_24Bit(unlock_detection_word);
	}

	LD_PinMode = LD_PinMode << 4;

	LD_PinMode_and_Precision = LD_PinMode | LD_Precision;

	LD_PinMode_and_Precision_word = Create24BitWord(LD_PinMode_and_Precision, REG_27h);

	sendWord_24Bit(LD_PinMode_and_Precision_word);


}














