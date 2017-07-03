/*

Author: Mason Edgar
Supervisor: John Sochacki
ViaSat Inc. Tempe, AZ

Slave Code For TivaC PLL Control Test

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


void setupClock(void);
void setupSPI(void);






int main(void) {
	
	uint32_t upper, lower, full_word;

	setupClock();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, 0);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 , 4);


	setupSPI();

	SSIDataGet(SSI0_BASE, &upper);
	SSIDataGet(SSI0_BASE, &lower);

	upper = upper << 16;

	full_word = upper | lower;

	if(full_word == 0xFFFFFFFF){
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2, 0);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3 , 8);
	}

	else{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, 0);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 , 2);
	}


	while(1){ };

	return 0;
}







void setupClock(void){

	// Set the system clock to 40 MHz
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

}



void setupSPI(void){

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
	MAP_SSIConfigSetExpClk(SSI0_BASE, MAP_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_SLAVE, 500000, 16);

	MAP_SSIEnable(SSI0_BASE);


}



