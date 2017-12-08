#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/pwm.h"

#include "cmsis_os.h"
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"

#include "buttons.h"

//buttons
//b1 - pin33 - PL1 - gpio 
//b2 - pin32 - PL2 - gpio  

/*_____________________________INIT_________________________________________*/
void button_init(){
	// Enable the GPIOA peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	// Wait for the GPIOA module to be ready.
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));

	// Initialize the GPIO pin configuration.
	// Set pins as input, SW controlled.
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE,GPIO_PIN_1); // Button 1 - PL1
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE,GPIO_PIN_2); // Button 2 - PL2
}

/*______________________________BUTTON 1____________________________________*/
bool button_read_s1(){
	return GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_1) ? false : true;
}

/*______________________________BUTTON 2____________________________________*/
bool button_read_s2(){
	return GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_2) ? false : true;
}
