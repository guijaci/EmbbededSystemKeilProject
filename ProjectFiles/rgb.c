//

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/pwm.h"

#include "rgb.h"

#ifndef __SysCtlClockGet
#define __SysCtlClockGet()	\
SysCtlClockFreqSet( 			\
	SYSCTL_XTAL_25MHZ	| 		\
	SYSCTL_OSC_MAIN 	| 		\
	SYSCTL_USE_PLL 		| 		\
	SYSCTL_CFG_VCO_480, 		\
	120000000)
#endif

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
#define PWMWidthPeriod(c, gen)  MAP_PWMGenPeriodGet(RGB_PWM_BASE, gen) * c / 0xFF
#define PWMWidthR(c)  					PWMWidthPeriod(c, RGB_PWM_GEN_R)
#define PWMWidthG(c)  					PWMWidthPeriod(c, RGB_PWM_GEN_G)
#define PWMWidthB(c)  					PWMWidthPeriod(c, RGB_PWM_GEN_B)
#define PWMDutyCicle(out, dc)  	MAP_PWMPulseWidthSet(RGB_PWM_BASE, out, dc)

static uint32_t g_ui32SysClock;

void rgb_write_r(uint8_t r){
	static bool enabled = false;
	if(r <  2 &&  enabled) {
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_R_BIT, false);
	}	
	PWMDutyCicle(RGB_PWM_OUT_R, PWMWidthR(r));
	if(r >= 2 && !enabled) { 
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_R_BIT, true);
	}
	enabled = r >= 2 ? true : false;
}

void rgb_write_g(uint8_t g){
	static bool enabled = false;
	if(g <  2 &&  enabled) {
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_G_BIT, false);
	}	
	PWMDutyCicle(RGB_PWM_OUT_G, PWMWidthG(g));
	if(g >= 2 && !enabled) {
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_G_BIT, true);
	}
	enabled = g >= 2 ? true : false;
}

void rgb_write_b(uint8_t b){
	static bool enabled = false;
	if(b <  2 &&  enabled) {
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_B_BIT, false);
	}
	PWMDutyCicle(RGB_PWM_OUT_B, PWMWidthB(b));
	if(b >= 2 && !enabled) {		
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_B_BIT, true);
	}
	enabled = b >= 2 ? true : false;
}

void rgb_write(uint8_t r, uint8_t g, uint8_t b){
	rgb_write_r(r);
	rgb_write_g(g);
	rgb_write_b(b);
}

void rgb_write_color(uint32_t rgb){
	rgb_write(
		rgb_color_r(rgb),
		rgb_color_g(rgb),
		rgb_color_b(rgb));
}

void rgb_init(){
	g_ui32SysClock = __SysCtlClockGet();
	
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));
	
	MAP_SysCtlPeripheralEnable(RGB_GPIOR_PERIPH);
	while(!MAP_SysCtlPeripheralReady(RGB_GPIOR_PERIPH));
	MAP_SysCtlPeripheralEnable(RGB_GPIOG_PERIPH);
	while(!MAP_SysCtlPeripheralReady(RGB_GPIOG_PERIPH));
	MAP_SysCtlPeripheralEnable(RGB_GPIOB_PERIPH);
	while(!MAP_SysCtlPeripheralReady(RGB_GPIOB_PERIPH));

	MAP_PWMGenDisable(RGB_PWM_BASE, RGB_PWM_GEN_R);
	MAP_PWMGenDisable(RGB_PWM_BASE, RGB_PWM_GEN_G);
	MAP_PWMGenDisable(RGB_PWM_BASE, RGB_PWM_GEN_B);
	
	MAP_GPIOPinTypePWM(RGB_LED_PORTR, RGB_LED_PINR);
	MAP_GPIOPinTypePWM(RGB_LED_PORTG, RGB_LED_PING);
	MAP_GPIOPinTypePWM(RGB_LED_PORTB, RGB_LED_PINB);

	MAP_PWMClockSet(RGB_PWM_BASE, PWM_SYSCLK_DIV_64);
	
	MAP_GPIOPinConfigure(RGB_PINCFG_R);
	MAP_GPIOPinConfigure(RGB_PINCFG_G);
	MAP_GPIOPinConfigure(RGB_PINCFG_B);

	MAP_PWMGenConfigure(RGB_PWM_BASE, RGB_PWM_GEN_R, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
	MAP_PWMGenConfigure(RGB_PWM_BASE, RGB_PWM_GEN_G, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
	MAP_PWMGenConfigure(RGB_PWM_BASE, RGB_PWM_GEN_B, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

	// Set the PWM period to 500Hz.  To calculate the appropriate parameter
	// use the following equation: N = (1 / f) * SysClk.  Where N is the
	// function parameter, f is the desired frequency, and SysClk is the
	// system clock frequency.
	// In this case you get: (1 / 500) * 120MHz / 64 = 3750 cycles.  Note that
	// the maximum period you can set is 2^16.
	// TODO: modify this calculation to use the clock frequency that you are
	// using.
	//
	MAP_PWMGenPeriodSet(RGB_PWM_BASE, RGB_PWM_GEN_R, g_ui32SysClock/PWM_FREQUENCY/64);
	MAP_PWMGenPeriodSet(RGB_PWM_BASE, RGB_PWM_GEN_G, g_ui32SysClock/PWM_FREQUENCY/64);
	MAP_PWMGenPeriodSet(RGB_PWM_BASE, RGB_PWM_GEN_B, g_ui32SysClock/PWM_FREQUENCY/64);
	
	rgb_write_color(RGB_OFF);

	MAP_PWMGenEnable (RGB_PWM_BASE, RGB_PWM_GEN_R); 	
	MAP_PWMGenEnable (RGB_PWM_BASE, RGB_PWM_GEN_G);
	MAP_PWMGenEnable (RGB_PWM_BASE, RGB_PWM_GEN_B);
}