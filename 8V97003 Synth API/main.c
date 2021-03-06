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
#include "Synth_API_Macro_Definitions.h"



//// Macro that sets the input array size to get from user
//#define MAX_INPUT_LENGTH 100
//
//// Macro for GetString function - this is the hex value for the enter key in ASCII
//#define ENTER 0x0D
//
//// Macros for selecting the chip-select polarity. The numbers here are arbitrary, the macro is for readability
//#define CS_ACTIVE_HIGH 0
//#define CS_ACTIVE_LOW  1


//// Masks to isolate bytes in the 24-bit words
//#define LOWER_8_BITS 0x000000FF
//#define MIDDLE_8_BITS 0x0000FF00
//#define UPPER_8_BITS 0x00FF0000
//
//// Masks to isolate bytes in 16-bit words
//#define TOP_HALF_16_BIT 0xFF00
//#define BOTTOM_HALF_16_BIT 0x00FF
//
//// Masks to isolate bytes in 32-bit words
//#define TOP_8_32Bit 0xFF000000
//#define UPPER_MIDDLE_32Bit 0x00FF0000
//#define LOWER_MIDDLE_32Bit 0x0000FF00
//#define BOTTOM_8_32Bit 0x000000FF
//
//// Macros to define lowest 8-bits (general usage)
//#define BIT_0 0x01
//#define BIT_1 0x02
//#define BIT_2 0x04
//#define BIT_3 0x08
//#define BIT_4 0x10
//#define BIT_5 0x20
//#define BIT_6 0x40
//#define BIT_7 0x80
//#define BIT3_to_BIT0 0x0F
//#define BIT6_to_BIT0 0x7F
//
//
//
//
///* Macros Specific to Register Programming */
//
//
//
//
//// These define the address for each register so that it can be combined with a data byte to form the full 24-bit frame.
//// All of the below register macros include the '0' MSB which indicates a write operation.
//
//
//#define REG_10h 0x1000
//#define REG_11h 0x1100
//#define REG_12h 0x1200
//#define REG_13h 0x1300
//#define REG_14h 0x1400
//#define REG_15h 0x1500
//#define REG_16h 0x1600
//#define REG_17h 0x1700
//#define REG_18h 0x1800
//#define REG_19h 0x1900
//#define REG_1Ah 0x1A00
//#define REG_1Bh 0x1B00
//#define REG_1Ch 0x1C00
//#define REG_1Dh 0x1D00
//#define REG_1Eh 0x1E00
//#define REG_1Fh 0x1F00
//#define REG_20h 0x2000
//#define REG_21h 0x2100
//#define REG_22h 0x2200
//#define REG_23h 0x2300
//#define REG_24h 0x2400
//#define REG_25h 0x2500
//#define REG_26h 0x2600
//#define REG_27h 0x2700
//#define REG_28h 0x2800
//#define REG_29h 0x2900
//#define REG_2Ah 0x2A00
//#define REG_2Bh 0x2B00
//#define REG_2Ch 0x2C00
//#define REG_2Dh 0x2D00
//#define REG_2Eh 0x2E00
//#define REG_2Fh 0x2F00
//#define REG_30h 0x3000
//#define REG_31h 0x3100
//#define REG_32h 0x3200
//#define REG_33h 0x3300
//#define REG_34h 0x3400
//#define REG_35h 0x3500
//#define REG_36h 0x3600
//#define REG_37h 0x3700
//#define REG_38h 0x3800
//#define REG_39h 0x3900
//#define REG_3Ah 0x3A00
//#define REG_3Bh 0x3B00
//#define REG_3Ch 0x3C00
//#define REG_3Dh 0x3D00
//#define REG_3Eh 0x3E00
//#define REG_3Fh 0x3F00
//#define REG_40h 0x4000
//#define REG_41h 0x4100
//#define REG_42h 0x4200
//#define REG_43h 0x4300
//
//// Read-Only Registers
//#define REG_44h_READ 0x804400
//#define REG_45h_READ 0x804500
//#define REG_46h_READ 0x804600
//#define REG_47h_READ 0x804700
//#define REG_48h_READ 0x804800
//
//// R/W Capability
//#define REG_49h_READ 0x804900
//#define REG_49h_WRITE 0x4900
//
//
//// Sets the Soft Reset pins in Register 0h. Further explained in the InitPrefaceRegister function comments
//#define SOFT_RESET 0x81
//
//// Sets the Read From Active Registers pin inside Register 1h. Further explained in the InitPrefaceRegister function comments
//#define READ_FROM_ACTIVE_REGISTERS 0x100
//
//// Charge Pump Register: High Impedance Macro
//#define CP_HIGH_Z 0x00000020
//
//// Output Control Register Macros
//#define QA_ENABLE_MUTE_UNTIL_LOCK_DETECT 0x38
//#define QA_ENABLE_NO_MUTE 0x18
//#define QB_ENABLE 0x18
//#define MUTE_UNTIL_LOCK_DETECT_WITHOUT_QA_ENABLED 0x28
//#define DISABLE_MUTE_WITHOUT_QA_ENABLED 0x08
//#define OutputDoublerEnable 0x80
//
//// Lock Detection Control Register Macros
//#define UNLOCK_DETECTION 0x08
//#define NO_UNLOCK_DETECTION 0x00


//
//// flag that makes sure chip select is asserted the correct way
//int cs_config;

//// global array for a quick newline write
//char newline[] = "\n\r";

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



void main(void){

}




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
 Changing AddressAscend to ON requires to set both AddressAscend and
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




void SetupInputControlRegisters(bool inputType, bool refDoublerEn, bool multEnable, bool nMultReset, unsigned long refDivider, unsigned long multRatio, unsigned long nMultPwrDwn){


/*

                                                                Input Control Register Map

  +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
  |                |                |                |                |                |                |                |                |                |
  |      ADDR      |       D7       |       D6       |      D5        |       D4       |       D3       |       D2       |       D1       |       D0       |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      0029      |     Ref<7>     |     Ref<6>     |     Ref<5>     |     Ref<4>     |     Ref<3>     |     Ref<2>     |     Ref<1>     |     Ref<0>     |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002A      |     UNUSED     |     UNUSED     |     UNUSED     |     UNUSED     |   InputType    | RefDoubler_En  |     Ref<9>     |     Ref<8>     |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002B      |    Mult_En     |  nMult_Reset   |     Mult<5>    |     Mult<4>    |     Mult<3>    |     Mult<2>    |     Mult<1>    |     Mult<0>    |
  |                |                |                |                |                |                |                |                |                |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                |                |                |                |                |                |                |                |                |
  |      002C      | nMultpwrdwn<2> | nMultpwrdwn<1> | nMultpwrdwn<0> |       0        |       0        |       1        |       0        |       0        |
  |                |                |                |                |                |                |                |                |                |
  +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


_____________________________________________________

 Ref[7:0]:

 Input reference divide value

 00 0000 0000 = Not Allowed
 00 0000 0001 = 1d (default)
 ...
 11 1111 1111 = 1023d

_____________________________________________________

 Input type:

 Selects either differential or single-ended input

 0 = Single-Ended Input
 1 = Differential Input (Default)

_____________________________________________________

 Reference Doubler Enable:

 Enables the Input Reference Doubler

 0 = Input reference doubler disabled
 1 = Input reference doubler enabled (default)

_____________________________________________________

 MULT Enable

 0 = MULT Not Enabled
 1 = MULT Enabled

_____________________________________________________

 nMult_reset:

 Resets the Reference multiplier block (MULT)

 0: Multiplier is reset (default)
 1: Multiplier is active

_____________________________________________________

 Mult[5:0]:

 Multiplication ratio for the multiplier block

 000000 = Unused
 000001 = Unused
 000010 = x2
 000011 = x3
 
 111111 = x63

_____________________________________________________

 nMultpwrdwn[2:0]:

 MULT power down

 000: MULT Powered Down
 001: Not Allowed
 
 110 = Not Allowed
 111 = MULT Powered Up

_____________________________________________________

*/

	unsigned long inputTypeValue, refDoublerEnValue, multEnableValue, nMultResetValue, nMultPwrDownshift, nMultPwrDownResult, nMultPwrDownWord;


	if(inputType){ inputTypeValue = BIT_3; }
	else if(!inputType){ inputTypeValue = 0; }

	if(refDoublerEn){ refDoublerEnValue = BIT_2; }
	else if(!refDoublerEn){ refDoublerEnValue = 0; }

	if(multEnable){ multEnableValue = BIT_7; }
	else if(!multEnable){ multEnableValue = 0; }

	if(nMultReset){ nMultResetValue = BIT_6; }
	else if(!nMultReset){ nMultResetValue = 0; }


	unsigned long refDividerBottom8Bits, refDividerTop2Bits, refDividerWord, refDividerResult, refDividerWord2;

	refDividerBottom8Bits = refDivider & BOTTOM_8_32Bit;

	refDividerTop2Bits = refDivider >> 8;

	refDividerWord = Create24BitWord(refDividerBottom8Bits, REG_29h);
	sendWord_24Bit(refDividerWord);

	refDividerResult = refDividerTop2Bits | inputTypeValue | refDoublerEnValue;

	refDividerWord2 = Create24BitWord(refDividerResult, REG_2Ah);
	sendWord_24Bit(refDividerWord2);

	unsigned long multResult, multResultWord;

	multResult = multEnableValue | nMultResetValue | multRatio;

	multResultWord = Create24BitWord(multResult, REG_2Bh);
	sendWord_24Bit(multResultWord);

	nMultPwrDownshift = nMultPwrDwn << 5;

	nMultPwrDownResult = nMultPwrDownshift | 0x04;

	nMultPwrDownWord = Create24BitWord(nMultPwrDownResult, REG_2Ch);
	sendWord_24Bit(nMultPwrDownWord);



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
  and the optional output divider M0, or to 2 × f_VCO (up to 18GHz) when using the optional output doubler, or an integer division of the VCO frequency f_VCO.
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
 101 = 6.4*
 110 = 10.4
 111 = 10.4*

 *The duplicate values are correct, according to the chip manufacturer.
          There are redundant settings for this parameter.
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







void SetPhaseAdjustment(unsigned long PhaseAdjustment){

/*


 The 8V97003 offers the possibility to adjust the phase delay between the outputs (RF_OUT/ nRF_OUT) and the input (REF_IN)
 of the device by shifting the output phase by a fraction of the size of the fractional denominator, when the part is used
 in fractional mode. Writing to the Phase Adjustment Control Registers triggers a phase shift (refer to below table).
 The phase adjustment value set by the bits, Phase[31:0], should be less than the fractional-N denominator register, MOD.
 The actual phase shift can be obtained with the following equation:




                                      Phase     T_pfd
 Phase Adjustment (degrees) =  360 X ------- X -------
                                       MOD      T_out


           OR

                                   Phase
 Phase Adjustment (ns) =  T_pfd X -------
                                    MOD



 Where 360° represents one cycle of output clock, T_pfd is the period at the input to the PFD (in ns), T_out is the period at the
 output of the 8V97003, and Phase is a programmable value, the same bit length as MOD.





                                                                        Phase Adjustment Register Map

  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       ADDR       |        D7        |        D6        |        D5        |        D4        |        D3        |        D2        |        D1        |        D0        |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       001A       |     Phase<7>     |     Phase<6>     |    Phase<5>      |    Phase<4>      |    Phase<3>      |    Phase<2>      |    Phase<1>      |    Phase<0>      |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       001B       |    Phase<15>     |    Phase<14>     |    Phase<13>     |    Phase<12>     |    Phase<11>     |    Phase<10>     |     Phase<9>     |     Phase<8>     |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       001C       |    Phase<23>     |    Phase<22>     |    Phase<21>     |    Phase<20>     |    Phase<19>     |    Phase<18>     |    Phase<17>     |    Phase<16>     |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       001D       |    Phase<31>     |    Phase<30>     |    Phase<29>     |    Phase<28>     |    Phase<27>     |    Phase<26>     |    Phase<25>     |    Phase<24>     |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+


*/

	unsigned long top, upper_middle, lower_middle, bottom;
	unsigned long top_word, upper_middle_word, lower_middle_word, bottom_word;

	SplitNumber_32Bit(PhaseAdjustment, &top, &upper_middle, &lower_middle, &bottom);

	bottom_word = Create24BitWord(bottom, REG_1Ah);
	lower_middle_word = Create24BitWord(lower_middle, REG_1Bh);
	upper_middle_word = Create24BitWord(upper_middle, REG_1Ch);
	top_word = Create24BitWord(top, REG_1Dh);

	sendWord_24Bit(bottom_word);
	sendWord_24Bit(lower_middle_word);
	sendWord_24Bit(upper_middle_word);
	sendWord_24Bit(top_word);


}



void CalibrationControl(bool ForceRelock, bool PhaseAdjustTrigger, bool BandSelectDisable, unsigned long BandSelectAccuracy){


/*

                                                                     Calibration Control Register Map

  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       ADDR       |        D7        |        D6        |        D5        |        D4        |        D3        |        D2        |        D1        |        D0        |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       0021       |   ForceRelock    |   PhaseAdjTrig   |  BandSelDisable  |      UNUSED      |        0         |        0         |   BandSelAcc<1>  |   BandSelAcc<0>  |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+


____________________________________________________________________________

 ForceRelock

 0 = Normal operation (Default)
 1 = VCO forced to recalibrate.

 This bit is self-clearing.

____________________________________________________________________________

 Phase Adjust Trigger

 0 = Normal operation
 1 = Trigger phase adjustment once.

 This bit is self-clearing.

____________________________________________________________________________

 Band Select Disable

 This bit will prevent a VCO recalibration when registers 16:25 are written
   - Registers 16:25 are the feedback divider values Nint, Nfrac, & Nmod

 0 = VCO recalibrates when registers 16:25 are written
 1 = VCO does not recalibrate when registers 16:25 are written

____________________________________________________________________________

Band select/Calibration Accuracy

00 = 1x resolution
01 = 2x resolution
10 = 4x resolution
11 = 8x resolution (default)

____________________________________________________________________________



*/


	unsigned long FR_Part, PAT_Part, BSD_Part, Calibration, Calibration_word;

	// To reduce code complexity and size, i'm checking all three booleans individually and OR-ing them together at the end



	if(ForceRelock){ FR_Part = 0x80; } // Set the ForceRelock bit
	else if(!ForceRelock){ FR_Part = 0x00; }

	if(PhaseAdjustTrigger){PAT_Part = 0x40; } // Set the Phase Adjust Trigger bit
	else if(!PhaseAdjustTrigger){PAT_Part = 0x00; }

	if(BandSelectDisable){BSD_Part = 0x20; } // Set the Band Disable bit
	else if(!BandSelectDisable){ BSD_Part = 0x00; }

	Calibration = FR_Part | PAT_Part | BSD_Part | BandSelectAccuracy; // Put them all together to form the data byte

	Calibration_word = Create24BitWord(Calibration, REG_21h);

	sendWord_24Bit(Calibration_word);



}



void SetCalibrationVoltage(unsigned long CaliVoltage){

/*


                                                                    Additional Calibration Control Register Map

  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       ADDR       |        D7        |        D6        |        D5        |        D4        |        D3        |        D2        |        D1        |        D0        |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  |       003C       |      UNUSED      |      UNUSED      |      UNUSED      |      UNUSED      |  CaliVoltage<3>  |  CaliVoltage<2>  |  CaliVoltage<1>  |  CaliVoltage<0>  |
  |                  |                  |                  |                  |                  |                  |                  |                  |                  |
  +------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------+


____________________________________________________________________

 Calibration Voltage Setting, sets Vc voltage during calibration

 0000 = 0.29V (default)
 0001 = 0.56V
 0010 = 0.83V
 0011 = 1.1V
 0100 = 1.37V
 0101 = 1.645V
 0110 = 1.915V
 0111 = 2.185V
 1000 = 2.455V
 1001 = 2.725V
 1010 = 3.0V

 1011 - 1111 = Unused

____________________________________________________________________


*/

	unsigned long CaliVoltage_word;

	CaliVoltage_word = Create24BitWord(CaliVoltage, REG_3Ch);

	sendWord_24Bit(CaliVoltage_word);



}




/*******************************************************************************************************/
/*     Takes in a status register keyword and send the status of that parameter back to the UART       */
/*******************************************************************************************************/


void ReadFromStatusRegisters(char* parameter){

/*
                                                    Status Register Map
    +------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+
    |      |             |             |             |             |             |             |             |             |
    | ADDR |     D7      |     D6      |     D5      |     D4      |     D3      |     D2      |     D1      |     D0      |
    |      |             |             |             |             |             |             |             |             |
    +----------------------------------------------------------------------------------------------------------------------+
    |      |             |             |             |             |             |             |             |             |
    | 0044 |   DigLock   | BandSelDone |   Unused    |   Unused    |  VcoSts<3>  |  VcoSts<2>  |  VcoSts<1>  |  VcoSts<0>  |
    |      |             |             |             |             |             |             |             |             |
    +----------------------------------------------------------------------------------------------------------------------+
    |      |             |             |             |             |             |             |             |             |
    | 0045 |   Unused    | BandSts<6>  | BandSts<5>  | BandSts<4>  | BandSts<3>  | BandSts<2>  | BandSts<1>  | BandSts<0>  |
    |      |             |             |             |             |             |             |             |             |
    +----------------------------------------------------------------------------------------------------------------------+
    |      |             |             |             |             |             |             |             |             |
    | 0046 |  Reserved   |  Reserved   |  Reserved   |  Reserved   |  Reserved   |  Reserved   |  Reserved   |  Reserved   |
    |      |             |             |             |             |             |             |             |             |
    +----------------------------------------------------------------------------------------------------------------------+
    |      |             |             |             |             |             |             |             |             |
    | 0047 |   Unused    |   Unused    |  Reserved   |  Reserved   | Reserved    |  Reserved   |  Reserved   |  Reserved   |
    |      |             |             |             |             |             |             |             |             |
    +----------------------------------------------------------------------------------------------------------------------+
    |      |             |             |             |             |             |             |             |             |
    | 0048 |   Unused    |   Unused    |   Unused    |   Unused    |   Unused    |   Unused    |   Unused    |   Unused    |
    |      |             |             |             |             |             |             |             |             |
    +----------------------------------------------------------------------------------------------------------------------+
    |      |             |             |             |             |             |             |             |             |
    | 0049 |   Unused    |   Unused    |   Unused    |   Unused    |   Unused    |   LossLock  |   Unused    |   Unused    |
    |      |             |             |             |             |             |             |             |             |
    +------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+-------------+

*/


	char temp[MAX_INPUT_LENGTH];
	char* temp_ptr = temp;

	unsigned long DigLockWord, BandSelectWord, currentVCO_Word, currentDigitalBandWord, lossOfLockWord;
	uint32_t DigLockValue, BandSelectValue, currentVCO_Value, currentDigitalBandValue, lossOfLockValue;

	clearArray(temp);

	// Copy character array into local workspace
	while(*parameter != '\0'){
		*temp_ptr = *parameter++;
		 temp_ptr++;
	}

	// Begin testing for particular commands
	if(!strncmp(temp, "DigLock", 7)){

		printString(newline);
		printString("Determining status of digital lock bit...");
		printString(newline);

		DigLockWord = Create24BitWord(0x00, REG_44h_READ);
		DigLockValue = GetRegisterValue(DigLockWord);

		// Isolate the Digital Lock bit
		DigLockValue &= BIT_7;

		if(DigLockValue){
			printString("PLL is locked.");
			printString(newline);
		}
		else{
			printString("PLL is NOT locked.");
			printString(newline);
		}
	}

	else if(!strncmp(temp, "BandSelectDone", 14)){

			printString(newline);
			printString("Determining if band selection has completed...");
			printString(newline);

			BandSelectWord = Create24BitWord(0x00, REG_44h_READ);
			BandSelectValue = GetRegisterValue(BandSelectWord);

			// Isolate the band selection done bit
			BandSelectValue &= BIT_6;

			if(BandSelectValue){
				printString("Band selection has completed.");
				printString(newline);
			}
			else{
				printString("Band selection has NOT completed.");
				printString(newline);
			}
	}

	else if(!strncmp(temp, "currentVCO", 10)){

			printString(newline);
			printString("Determining which VCO is active...");
			printString(newline);

			currentVCO_Word = Create24BitWord(0x00, REG_44h_READ);
			currentVCO_Value = GetRegisterValue(currentVCO_Word);

			// Isolate the VCO value bits
			currentVCO_Value &= BIT3_to_BIT0;

			printString("Current active VCO: VCO #");
			printInt(currentVCO_Value);
			printString(newline);
	}

	else if(!strncmp(temp, "currentDigitalBand", 18)){

			printString(newline);
			printString("Determining which digital band is active...");
			printString(newline);

			currentDigitalBandWord = Create24BitWord(0x00, REG_45h_READ);
			currentDigitalBandValue = GetRegisterValue(currentDigitalBandWord);

			// Isolate the digital band value bits
			currentDigitalBandValue &= BIT6_to_BIT0;

			printString("Current active digital band: Band ");
			printInt(currentDigitalBandValue);
			printString(newline);
	}

	else if(!strncmp(temp, "lossOfLock", 10)){

			printString(newline);
			printString("Determining whether digital lock has been lost...");
			printString(newline);

			lossOfLockWord = Create24BitWord(0x00, REG_49h_READ);
			lossOfLockValue = GetRegisterValue(lossOfLockWord);

			// Isolate the loss of digital lock bit
			lossOfLockValue &= BIT_2;

			if(lossOfLockValue){
				printString("Digital lock has been lost.");
				printString(newline);

				// Sticky bit; must write a '1' to clear
				lossOfLockWord = Create24BitWord(BIT_2, REG_49h_WRITE);
				sendWord_24Bit(lossOfLockWord);
			}
			else{
				printString("Digital lock is still active.");
				printString(newline);
			}
	}


}


void setVcoResolution(unsigned long bias_value){

/*

                                         VCO Resolution Control Register Map
  +------+------------+------------+------------+------------+------------+------------+------------+------------+
  |      |            |            |            |            |            |            |            |            |
  | ADDR |     D7     |     D6     |     D5     |     D4     |     D3     |     D2     |     D1     |     D0     |
  |      |            |            |            |            |            |            |            |            |
  +--------------------------------------------------------------------------------------------------------------+
  |      |            |            |            |            |            |            |            |            |
  | 0030 | vco_dac<7> | vco_dac<6> | vco_dac<5> | vco_dac<4> | vco_dac<3> | vco_dac<2> | vco_dac<1> | vco_dac<0> |
  |      |            |            |            |            |            |            |            |            |
  +------+------------+------------+------------+------------+------------+------------+------------+------------+


 VCO Bias DAC Setting (Resolution = 6.18mV / step)

 0000 0000 = 489 mV
 0000 0001 = 495.2 mV
 0000 0010 = 501.4 mV
 
 0110 0010 = 1.095 V (default)
 
 1111 1111 = 2.067 V

*/


	unsigned long VcoResolutionWord;

	VcoResolutionWord = Create24BitWord(bias_value, REG_30h);

	sendWord_24Bit(VcoResolutionWord);

}



void controlDSM(bool DSMtype, bool ShapeDitherEn, bool DitherEn, unsigned long DSMorder, unsigned long DitherGain){

/*

                                                       DSM Control Register Map (Delta Sigma Modulator)
  +---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+
  |               |               |               |               |               |               |               |               |               |
  |     ADDR      |      D7       |      D6       |      D5       |      D4       |      D3       |      D2       |      D1       |      D0       |
  |               |               |               |               |               |               |               |               |               |
  +-----------------------------------------------------------------------------------------------------------------------------------------------+
  |               |               |               |               |               |               |               |               |               |
  |     001E      |    DSMType    |  DSMOrder<2>  |  DSMOrder<1>  |  DSMOrder<0>  |  DitherG<1>   |  DitherG<0>   | ShapeDitherEn |   DitherEn    |
  |               |               |               |               |               |               |               |               |               |
  +---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+


 In order to reduce the spurs, the user can enable the dither function to increase the repeat length of the code sequence in the Sigma Delta Modulator (DSM).
 The increased repeat length is 2^32 - 1 cycles so that the resulting quantization error is spread to appear like broadband noise. As a result, the
 in-band phase noise may be degraded when using the dither function. When the application requires the lowest possible phase noise and when the loop
 bandwidth is low enough to filter most of the undesirable spurs, or if the spurs wont affect the system performance, it is recommended to use the low
 noise mode with dither disabled.



*/


	unsigned long DSMvalue, ShapeDitherEnValue, DitherEnValue, DSMshift, DitherGainshift, DSMresult, DSMword;

	if(DSMtype){ DSMvalue = BIT_7; }
	else if(!DSMtype){ DSMvalue = 0; }

	if(ShapeDitherEn){ ShapeDitherEnValue = BIT_1; }
	else if(!ShapeDitherEn){ ShapeDitherEnValue = 0; }

	if(DitherEn){ DitherEnValue = BIT_0; }
	else if(!DitherEn){ DitherEnValue = 0; }

	DSMshift = DSMorder << 4;

	DitherGainshift = DitherGain << 2;

	DSMresult = DSMvalue | ShapeDitherEnValue | DitherEnValue | DSMshift | DitherGainshift;

	DSMword = Create24BitWord(DSMresult, REG_1Eh);

	sendWord_24Bit(DSMword);

}
