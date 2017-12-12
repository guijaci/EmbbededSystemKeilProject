//..............................................................................
// rgb.c - Driver for interfacing an RGB LED.
//
// Copyright (c) 2017 
// Allan Patrick de Souza - <allansouza@alunos.utfpr.edu.br>
// Guilherme Jacichen     - <jacichen@alunos.utfpr.edu.br>
// Jessica Isoton Sampaio - <jessicasampaio@alunos.utfpr.edu.br>
// Mariana Carrião        - <mcarriao@alunos.utfpr.edu.br>
//
// All rights reserved. 
// Software License Agreement
//...............................................................................

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/pwm.h"

#include "rgb.h"

/*******************************************************************************
 * @brief Redefining this macro overwrite the default method for getting system 
 *				clock. Tiva TM4C1294 systems can't use the TivaWare's SysCtlClockGet() 
 *				to get the system clock, so SysCtlClockFreqSet() is used instead.
 *******************************************************************************/
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
// Defines the PWM adn GPIO peripherals that are used for this LED RGB.
//
//*****************************************************************************
#define RGB_PWM_PERIPH		SYSCTL_PERIPH_PWM0
#define RGB_GPIOR_PERIPH	SYSCTL_PERIPH_GPIOF
#define RGB_GPIOG_PERIPH	SYSCTL_PERIPH_GPIOF
#define RGB_GPIOB_PERIPH	SYSCTL_PERIPH_GPIOG

//*****************************************************************************
//
// Defines the GPIO pin configuration macros for the pins that are used for
// the PWM function.
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

//*****************************************************************************
//
// Defines for the PWM hardware configuration (generators and pin output)
//
//*****************************************************************************
#define RGB_PWM_GEN_R			PWM_GEN_1
#define RGB_PWM_GEN_G			PWM_GEN_1
#define RGB_PWM_GEN_B			PWM_GEN_2

#define RGB_PWM_OUT_R			PWM_OUT_2
#define RGB_PWM_OUT_G			PWM_OUT_3
#define RGB_PWM_OUT_B			PWM_OUT_4

#define RGB_PWM_OUT_R_BIT	PWM_OUT_2_BIT
#define RGB_PWM_OUT_G_BIT	PWM_OUT_3_BIT
#define RGB_PWM_OUT_B_BIT	PWM_OUT_4_BIT

//*****************************************************************************
//
// Defines the RGB PWM frequency
//
//*****************************************************************************
#define PWM_FREQUENCY			500

//*****************************************************************************
//
// Defines macros to simplify PWM duty cycle modification
//
//*****************************************************************************

/******************************************************************************
 * @brief Returns desired pulse length period.
 * @param  c is desired light intensity (16bit, 0x0000 is minimum and 0xFFFF is maximum)
 * @param  gen is desired PWM to convert the pulse length
 ******************************************************************************/
#define PWMWidthPeriod(c, gen)  MAP_PWMGenPeriodGet(RGB_PWM_BASE, gen) * c / 0xFF

/******************************************************************************
 * @brief Returns desired pulse length period for the red component of the LED RGB.
 * @param  c is desired light intensity (16bit, 0x0000 is minimum and 0xFFFF is maximum)
 ******************************************************************************/
#define PWMWidthR(c)  					PWMWidthPeriod(c, RGB_PWM_GEN_R)

/******************************************************************************
 * @brief Returns desired pulse length period for the green component of the LED RGB.
 * @param  c is desired light intensity (16bit, 0x0000 is minimum and 0xFFFF is maximum)
 ******************************************************************************/
#define PWMWidthG(c)  					PWMWidthPeriod(c, RGB_PWM_GEN_G)

/******************************************************************************
 * @brief Returns desired pulse length period for the blue component of the LED RGB.
 * @param  c is desired light intensity (16bit, 0x0000 is minimum and 0xFFFF is maximum)
 ******************************************************************************/
#define PWMWidthB(c)  					PWMWidthPeriod(c, RGB_PWM_GEN_B)

/******************************************************************************
 * @brief Sets the PWM duty cicle period of one LED RGB component.
 * @param  out is desired LED RGB component output symbol to modify the duty cycle
 * @param  dc is pulse length period of the duty cycle
 ******************************************************************************/
#define PWMDutyCicle(out, dc)  	MAP_PWMPulseWidthSet(RGB_PWM_BASE, out, dc)

static uint32_t g_ui32SysClock;

/*******************************************************************************
 * @brief Writes the red component intensity value of the LED RGB.
 * @param  r is the 8bit red intensity value, where 0x00 is the 
 * 				 minimum and 0xFF is the maximum.
 ******************************************************************************/
void rgb_write_r(uint8_t r){
	static bool enabled = false;
	// if the value is lesser than 2, disable the PWM output to pin (avoids bright led pulses)
	if(r <  2 &&  enabled) {
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_R_BIT, false);
	}	
	PWMDutyCicle(RGB_PWM_OUT_R, PWMWidthR(r));
	// if the value is more than 2, enables again the PWM output to pin
	if(r >= 2 && !enabled) { 
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_R_BIT, true);
	}
	// enables flag avoids calling same function multiple times
	enabled = r >= 2 ? true : false;
}

/*******************************************************************************
 * @brief Writes the green component intensity value of the LED RGB.
 * @param  g is the 8bit green intensity value, where 0x00 is the 
 * 				 minimum and 0xFF is the maximum.
 ******************************************************************************/
void rgb_write_g(uint8_t g){
	static bool enabled = false;
	// if the value is lesser than 2, disable the PWM output to pin (avoids bright led pulses)
	if(g <  2 &&  enabled) {
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_G_BIT, false);
	}	
	PWMDutyCicle(RGB_PWM_OUT_G, PWMWidthG(g));
	// if the value is more than 2, enables again the PWM output to pin
	if(g >= 2 && !enabled) {
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_G_BIT, true);
	}
	// enables flag avoids calling same function multiple times
	enabled = g >= 2 ? true : false;
}

/*******************************************************************************
 * @brief Writes the blue component intensity value of the LED RGB.
 * @param  b is the 8bit blue intensity value, where 0x00 is the 
 * 				 minimum and 0xFF is the maximum.
 ******************************************************************************/
void rgb_write_b(uint8_t b){
	static bool enabled = false;
	// if the value is lesser than 2, disable the PWM output to pin (avoids bright led pulses)
	if(b <  2 &&  enabled) {
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_B_BIT, false);
	}
	// modify the pulse width period
	PWMDutyCicle(RGB_PWM_OUT_B, PWMWidthB(b));
	// if the value is more than 2, enables again the PWM output to pin
	if(b >= 2 && !enabled) {		
		MAP_PWMOutputState(RGB_PWM_BASE, RGB_PWM_OUT_B_BIT, true);
	}
	// enables flag avoids calling same function multiple times
	enabled = b >= 2 ? true : false;
}

/*******************************************************************************
 * @brief Writes the red, green and blue components intensity values of the LED RGB.
 * @param  r is the 8bit red intensity value, where 0x00 is the 
 * 				 minimum and 0xFF is the maximum.
 * @param  g is the 8bit green intensity value, where 0x00 is the 
 * 				 minimum and 0xFF is the maximum.
 * @param  b is the 8bit blue intensity value, where 0x00 is the 
 * 				 minimum and 0xFF is the maximum.
 ******************************************************************************/
void rgb_write(uint8_t r, uint8_t g, uint8_t b){
	rgb_write_r(r);
	rgb_write_g(g);
	rgb_write_b(b);
}

/*******************************************************************************
 * @brief Writes a 24bits color value to modify the components intensity 
 *				values of the LED RGB.
 * @param  rgb is the 24bits color intensity value, with the format in hex 
 *				 00RRGGBB - R are the red byte, G the green byte and B the blue byte 
 *				 (00 is ignored)
 ******************************************************************************/
void rgb_write_color(uint32_t rgb){
	rgb_write(
		rgb_color_r(rgb),
		rgb_color_g(rgb),
		rgb_color_b(rgb));
}

/*******************************************************************************
 * @brief Correctly initializes register configurations 
 *				for using the Servo Motor's PWM.
 *******************************************************************************/
void rgb_init(){
	g_ui32SysClock = __SysCtlClockGet();
	
	// Enables the PWM's and GPIO peripherals
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

	MAP_SysCtlPeripheralEnable(RGB_GPIOR_PERIPH);
	MAP_SysCtlPeripheralEnable(RGB_GPIOG_PERIPH);
	MAP_SysCtlPeripheralEnable(RGB_GPIOB_PERIPH);
	
	// Waits the enabling operation complete
	while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0) &&
				!MAP_SysCtlPeripheralReady(RGB_GPIOR_PERIPH) &&
				!MAP_SysCtlPeripheralReady(RGB_GPIOG_PERIPH) &&
				!MAP_SysCtlPeripheralReady(RGB_GPIOB_PERIPH));

	// Disable the generators, in case of a software reset
	MAP_PWMGenDisable(RGB_PWM_BASE, RGB_PWM_GEN_R);
	MAP_PWMGenDisable(RGB_PWM_BASE, RGB_PWM_GEN_G);
	MAP_PWMGenDisable(RGB_PWM_BASE, RGB_PWM_GEN_B);
	
	// Configure each PWM pin type as PWM
	MAP_GPIOPinTypePWM(RGB_LED_PORTR, RGB_LED_PINR);
	MAP_GPIOPinTypePWM(RGB_LED_PORTG, RGB_LED_PING);
	MAP_GPIOPinTypePWM(RGB_LED_PORTB, RGB_LED_PINB);

	// Configures PWM clock and dividers
	MAP_PWMClockSet(RGB_PWM_BASE, PWM_SYSCLK_DIV_64);
	
	// Configure each PWM pin to use the pwm
	MAP_GPIOPinConfigure(RGB_PINCFG_R);
	MAP_GPIOPinConfigure(RGB_PINCFG_G);
	MAP_GPIOPinConfigure(RGB_PINCFG_B);

	// Configure each PWM generator in basic mode (no sync, mode down)
	MAP_PWMGenConfigure(RGB_PWM_BASE, RGB_PWM_GEN_R, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
	MAP_PWMGenConfigure(RGB_PWM_BASE, RGB_PWM_GEN_G, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
	MAP_PWMGenConfigure(RGB_PWM_BASE, RGB_PWM_GEN_B, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

	// Set the PWM period to 500Hz.  To calculate the appropriate parameter
	// use the following equation: N = (1 / f) * SysClk.  Where N is the
	// function parameter, f is the desired frequency, and SysClk is the
	// system clock frequency.
	// In this case you get: (1 / 500) * 120MHz / 64 = 3750 cycles.  Note that
	// the maximum period you can set is 2^16.
	MAP_PWMGenPeriodSet(RGB_PWM_BASE, RGB_PWM_GEN_R, g_ui32SysClock/PWM_FREQUENCY/64);
	MAP_PWMGenPeriodSet(RGB_PWM_BASE, RGB_PWM_GEN_G, g_ui32SysClock/PWM_FREQUENCY/64);
	MAP_PWMGenPeriodSet(RGB_PWM_BASE, RGB_PWM_GEN_B, g_ui32SysClock/PWM_FREQUENCY/64);
	
	// Shuts off the lights
	rgb_write_color(RGB_OFF);

	// Enables each generator again
	MAP_PWMGenEnable (RGB_PWM_BASE, RGB_PWM_GEN_R); 	
	MAP_PWMGenEnable (RGB_PWM_BASE, RGB_PWM_GEN_G);
	MAP_PWMGenEnable (RGB_PWM_BASE, RGB_PWM_GEN_B);
}