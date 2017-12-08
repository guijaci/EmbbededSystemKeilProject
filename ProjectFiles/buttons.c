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
void buttons_init(){
	// Enable the GPIOA peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	// Wait for the GPIOA module to be ready.
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC)){
	}

	// Initialize the GPIO pin configuration.
	// Set pins as input, SW controlled.
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE,GPIO_PIN_1); // Button 1 - PL1
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE,GPIO_PIN_2); // Button 2 - PL2
}

/*______________________________BUTTON 1____________________________________*/
void button1_read(){
	int32_t i32Button1;
	int button1pressed;
	
	GPIOIntTypeSet(GPIO_PORTL_BASE, GPIO_PIN_1, GPIO_HIGH_LEVEL);
	
	// Read some pins.
	i32Button1 = GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_1);
	
	// Enable the pin interrupts.
	GPIOIntEnable(GPIO_PORTL_BASE, GPIO_PIN_1);

	while(1){
	 i32Button1 = 0x00;
	 i32Button1 = GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_1);
	
	 button1pressed = 1;
		
		if(i32Button1 == 0){
			button1pressed = 0;
		}	
	}

	
}

/*______________________________BUTTON 2____________________________________*/
void button2_read(){
	int32_t i32Button2;
	int button2pressed;
	
	GPIOIntTypeSet(GPIO_PORTL_BASE, GPIO_PIN_2, GPIO_HIGH_LEVEL);
	
	// Read some pins.
	i32Button2 = GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_2);
	
	// Enable the pin interrupts.
	GPIOIntEnable(GPIO_PORTL_BASE, GPIO_PIN_2);
	
	while(1){
	 i32Button2 = 0x00;
	 i32Button2 = GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_2);
		
	 button2pressed = 1;
		
		if(i32Button2 == 0){
			button2pressed = 0;
		}
	}
	
}


