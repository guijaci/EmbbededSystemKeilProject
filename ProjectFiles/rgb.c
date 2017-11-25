//

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

#include "rgb.h"

//*****************************************************************************
//
// Defines the PWM adn GPIO peripherals that are used for this RGB LED.
//
//*****************************************************************************
#define RGB_PWM_PERIPH		SYSCTL_PERIPH_PWM0
#define RGB_GPIOR_PERIPH	SYSCTL_PERIPH_GPIOF
#define RGB_GPIOG_PERIPH	SYSCTL_PERIPH_GPIOF
#define RGB_GPIOB_PERIPH	SYSCTL_PERIPH_GPIOG
//#define RGB_GPIO2_PERIPH

//*****************************************************************************
//
// Defines the GPIO pin configuration macros for the pins that are used for
// the SSI function.
//
//*****************************************************************************
#define RGB_PINCFG_R			GPIO_PF2_M0PWM2
#define RGB_PINCFG_G			GPIO_PF3_M0PWM3
#define RGB_PINCFG_B			GPIO_PG0_M0PWM4

//*****************************************************************************
//
// Defines the port and pins for the RGB peripheral.
//
//*****************************************************************************
#define RGB_PWM_BASE			PWM0_BASE

#define RGB_LED_PORTR			GPIO_PORTF_BASE
#define RGB_LED_PINR			GPIO_PIN_2

#define RGB_LED_PORTG			GPIO_PORTF_BASE
#define RGB_LED_PING			GPIO_PIN_3

#define RGB_LED_PORTB			GPIO_PORTG_BASE
#define RGB_LED_PINB			GPIO_PIN_0

//

#define RGB_PWM_GEN_R			PWM_GEN_1
#define RGB_PWM_GEN_G			PWM_GEN_1
#define RGB_PWM_GEN_B			PWM_GEN_2

#define RGB_PWM_OUT_R			PWM_OUT_2
#define RGB_PWM_OUT_G			PWM_OUT_3
#define RGB_PWM_OUT_B			PWM_OUT_4

#define RGB_PWM_OUT_R_BIT	PWM_OUT_2_BIT
#define RGB_PWM_OUT_G_BIT	PWM_OUT_3_BIT
#define RGB_PWM_OUT_B_BIT	PWM_OUT_4_BIT


#define PWM_FREQUENCY			500

//
#define PWMWidthPeriod(c, gen)  PWMGenPeriodGet(RGB_PWM_BASE, gen) * c / 0xFF
#define PWMWidthR(c)  					PWMWidthPeriod(c, RGB_PWM_GEN_R)
#define PWMWidthG(c)  					PWMWidthPeriod(c, RGB_PWM_GEN_G)
#define PWMWidthB(c)  					PWMWidthPeriod(c, RGB_PWM_GEN_B)
#define PWMDutyCicle(out, dc)  	PWMPulseWidthSet(RGB_PWM_BASE, out, dc)

uint32_t ui32SysClock;

void 
rgb_write_r(uint8_t r){
	static bool enabled = false;
	if(!r &&  enabled) {
		ROM_PWMGenDisable(RGB_PWM_BASE, RGB_PWM_GEN_R);
		ROM_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_R_BIT, false);
	}	
	PWMDutyCicle(RGB_PWM_OUT_R, PWMWidthR(r));
	if( r && !enabled) { 
		ROM_PWMGenEnable (RGB_PWM_BASE, RGB_PWM_GEN_R); 	
		ROM_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_R_BIT, true);
	}
	enabled = r ? true : false;
}

void 
rgb_write_g(uint8_t g){
	static bool enabled = false;
	if(!g &&  enabled) {
		ROM_PWMGenDisable(RGB_PWM_BASE, RGB_PWM_GEN_G);
		ROM_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_G_BIT, false);
	}	
	PWMDutyCicle(RGB_PWM_OUT_G, PWMWidthG(g));
	if( g && !enabled) {
		ROM_PWMGenEnable (RGB_PWM_BASE, RGB_PWM_GEN_G);
		ROM_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_G_BIT, true);
	}
	enabled = g ? true : false;
}

void 
rgb_write_b(uint8_t b){
	static bool enabled = false;
	if(!b &&  enabled) {
		ROM_PWMGenDisable(RGB_PWM_BASE, RGB_PWM_GEN_B);
		ROM_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_B_BIT, false);
	}
	PWMDutyCicle(RGB_PWM_OUT_B, PWMWidthB(b));
	if( b && !enabled) {
		ROM_PWMGenEnable (RGB_PWM_BASE, RGB_PWM_GEN_B);
		ROM_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_B_BIT, true);
	}
	enabled = b ? true : false;
}

void
rgb_write(uint8_t r, uint8_t g, uint8_t b){
	rgb_write_r(r);
	rgb_write_g(g);
	rgb_write_b(b);
}

void
rgb_write_c24b(uint32_t rgb){
	rgb_write(
		((rgb & 0x00FF0000) >> 16 ),
		((rgb & 0x0000FF00) >>  8 ),
		 (rgb & 0x000000FF)       );
}

void 
rgb_init(){
	ui32SysClock = 120000000;
	
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));
	
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
	ROM_SysCtlPeripheralEnable(RGB_GPIOR_PERIPH);
	while(!ROM_SysCtlPeripheralReady(RGB_GPIOR_PERIPH));
	ROM_SysCtlPeripheralEnable(RGB_GPIOG_PERIPH);
	while(!ROM_SysCtlPeripheralReady(RGB_GPIOG_PERIPH));
	ROM_SysCtlPeripheralEnable(RGB_GPIOB_PERIPH);
	while(!ROM_SysCtlPeripheralReady(RGB_GPIOB_PERIPH));

	//HWREG(GPIO_PORTF_AHB_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;
	//HWREG(GPIO_PORTF_AHB_BASE+GPIO_O_CR) 	|= GPIO_PIN_0;

	//GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_0);
	ROM_GPIOPinTypePWM(RGB_LED_PORTR, RGB_LED_PINR);
	ROM_GPIOPinTypePWM(RGB_LED_PORTG, RGB_LED_PING);
	ROM_GPIOPinTypePWM(RGB_LED_PORTB, RGB_LED_PINB);

	ROM_PWMClockSet(RGB_PWM_BASE, PWM_SYSCLK_DIV_64);
	
	//GPIOPinConfigure(GPIO_PF0_M0PWM0);
	ROM_GPIOPinConfigure(RGB_PINCFG_R);
	ROM_GPIOPinConfigure(RGB_PINCFG_G);
	ROM_GPIOPinConfigure(RGB_PINCFG_B);

	//PWMGenConfigure(PWM0_BASE, PWM_GEN_0, (PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC));
	ROM_PWMGenConfigure(RGB_PWM_BASE, RGB_PWM_GEN_R, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
	ROM_PWMGenConfigure(RGB_PWM_BASE, RGB_PWM_GEN_G, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
	ROM_PWMGenConfigure(RGB_PWM_BASE, RGB_PWM_GEN_B, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

	// Set the PWM period to 500Hz.  To calculate the appropriate parameter
	// use the following equation: N = (1 / f) * SysClk.  Where N is the
	// function parameter, f is the desired frequency, and SysClk is the
	// system clock frequency.
	// In this case you get: (1 / 500) * 120MHz / 64 = 3750 cycles.  Note that
	// the maximum period you can set is 2^16.
	// TODO: modify this calculation to use the clock frequency that you are
	// using.
	//
	//PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 3750);
	ROM_PWMGenPeriodSet(RGB_PWM_BASE, RGB_PWM_GEN_R, 3750);
	ROM_PWMGenPeriodSet(RGB_PWM_BASE, RGB_PWM_GEN_G, 3750);
	ROM_PWMGenPeriodSet(RGB_PWM_BASE, RGB_PWM_GEN_B, 3750);
	
	//PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
	//ROM_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_R_BIT, true);
	//ROM_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_G_BIT, true);
	//ROM_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_B_BIT, true);

	//PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,  0);
	//ROM_PWMPulseWidthSet(RGB_PWM_BASE, RGB_PWM_OUT_R, 0);
	//ROM_PWMPulseWidthSet(RGB_PWM_BASE, RGB_PWM_OUT_G, 0);
	//ROM_PWMPulseWidthSet(RGB_PWM_BASE, RGB_PWM_OUT_B, 0);
	
	//PWMGenEnable(PWM0_BASE, PWM_GEN_0);
	//ROM_PWMGenEnable(RGB_PWM_BASE, RGB_PWM_GEN_R);
	//ROM_PWMGenEnable(RGB_PWM_BASE, RGB_PWM_GEN_G);
	//ROM_PWMGenEnable(RGB_PWM_BASE, RGB_PWM_GEN_B);
	
	rgb_write_c24b(RGB_OFF);
}