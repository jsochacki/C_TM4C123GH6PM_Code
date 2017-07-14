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
#include "driverlib/pwm.h"
#include "driverlib/ssi.h"
#include "utils/uartstdio.h"


#define CS_ACTIVE_HIGH 0
#define CS_ACTIVE_LOW  1
#define UPPER_16_BITS 0xFFFF0000
#define LOWER_16_BITS 0x0000FFFF

// Masks to isolate bytes in the 24-bit words
#define LOWER_8_BITS 0x000000FF
#define MIDDLE_8_BITS 0x0000FF00
#define UPPER_8_BITS 0x00FF0000

//Macro for GetString
#define ENTER 0x0D

// flag that makes sure chip select is asserted the correct way
int cs_config;

// Macro that sets the input array size to get from user
#define MAX_INPUT_LENGTH 100



void setupClock(void);
void setupSPI_16Bit(void);
void setupChipSelect(int mode);
void sendWord(unsigned long word);
void sendWord_24Bit(unsigned long word);
void setupSPI_8Bit(void);
void printSingleNumber(int num);
void printInt(int value);
void printString(char *string1);
void InitConsole(void);
void getString(char* user_string);
void ClearUserInput(char* input);
uint32_t readWord_24Bit(unsigned long word);
uint32_t GetRegisterValue(unsigned long word);


int main(void) {
	
	setupClock();
	InitConsole();
	setupChipSelect(CS_ACTIVE_LOW);
	setupSPI_8Bit();
	unsigned long address = 0x123450;

	uint32_t registerValue;

	registerValue = GetRegisterValue(address);

	printString("Return Value: ");
	printInt(registerValue);
	printString("\n\r");


	while(1){ };



	return 0;
}


uint32_t GetRegisterValue(unsigned long address){

	uint32_t regData;
	int i;

	for(i=0; i<2; i++){ regData = readWord_24Bit(address); }

	return regData;
}



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

	MAP_SSIDataGet(SSI0_BASE, &returnValue);

	// Close the frame by de-asserting chip select
	if(cs_config == 1){ MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00); }
	else if(cs_config == 0) { MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x2); }


	return returnValue;

}



void ClearUserInput(char* input){

	int i;

	for(i = 0; i < MAX_INPUT_LENGTH; i++){ *input++ = '\0'; }

}




void getString(char* user_string){

	int count = 0;

	while( (*user_string = MAP_UARTCharGet(UART0_BASE)) != ENTER ){

		MAP_UARTCharPut(UART0_BASE, *user_string++);
		if(++count == MAX_INPUT_LENGTH){ break; }

	}

}


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


void printString(char *string){

    uint32_t ui32Counter = 0;

    while (string[ui32Counter] != '\0'){

        MAP_UARTCharPut(UART0_BASE, string[ui32Counter]);
        ui32Counter = ui32Counter + 1;
    }
}


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


void setupClock(void){

	// Set the system clock to 40 MHz
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

}




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



void sendWord(unsigned long word){

	// Break 32-bit word into 2 16-bit words
	uint32_t upper, lower;
	unsigned long temp1, temp2;

	temp1 = word & UPPER_16_BITS;
	temp2 = word & LOWER_16_BITS;

	upper = temp1 >> 16;
	lower = temp2;

	// Enable chip select to start transfer
	if(cs_config == 1){ MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x2); }
	else if(cs_config == 0) { MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00); }

	// Send both halves of the original word, MSB first
	MAP_SSIDataPut(SSI0_BASE, upper);
	MAP_SSIDataPut(SSI0_BASE, lower);

	while(MAP_SSIBusy(SSI0_BASE)){};

	// Close the frame by de-asserting chip select
	if(cs_config == 1){ MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00); }
	else if(cs_config == 0) { MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x2); }

}

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

void setupSPI_16Bit(void){

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
	MAP_SSIConfigSetExpClk(SSI0_BASE, MAP_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 500000, 16);

	MAP_SSIEnable(SSI0_BASE);

}
