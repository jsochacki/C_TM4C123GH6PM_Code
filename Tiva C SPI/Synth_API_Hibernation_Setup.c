/*
 * Synth_API_Hibernation_Setup.c
 *
 *  Author: Mason Edgar
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
#include "driverlib/hibernate.h"
#include "Synth_API_Hibernation_Setup.h"



void initHibernationModule(void){

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x04);

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
	MAP_HibernateEnableExpClk(MAP_SysCtlClockGet());
	MAP_HibernateGPIORetentionEnable();
	MAP_HibernateWakeSet(HIBERNATE_WAKE_PIN);

}