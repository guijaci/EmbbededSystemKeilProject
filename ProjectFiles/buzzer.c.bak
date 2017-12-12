//..............................................................................
// buzzer.c - Driver for the Piezo Buzzer
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

// pin40 - buzzer out - PF1 - M0PWM1 -- PWM out

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"

#include "buzzer.h"

#define PWM_FREQUENCY			500

static uint32_t g_ui32SysClock;
static uint32_t g_current_vol;
static uint16_t g_current_per;
static bool g_current_state;

typedef enum {FREQ_0, FREQ_1, FREQ_2, FREQ_3, FREQ_4, FREQ_5, FREQ_6} 
freq_t;

/***************************************************************************//**
 * @brief Set values of different clocks to FREQ_0, FREQ_1, ...., FREQ_6 
 ******************************************************************************/
static void 
buzzer_div_set(freq_t div_freq){	
	switch(div_freq){
		case FREQ_0:
			PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_64);			
		break;
		case FREQ_1:
			PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_32);
		break;
		case FREQ_2:
			PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_16);
		break;
		case FREQ_3:
			PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_8);
		break;
		case FREQ_4:
				PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_4);
		break;
		case FREQ_5:
				PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_2);
		break;
		case FREQ_6:
				PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_1);
	}
}

/***************************************************************************//**
 * @brief Set the pin as output 
 * @param channel to be read to get the value from the pin.
 * @return result[0] the voltage value corresponded to the channel.
 ******************************************************************************/
void
buzzer_write(bool enable){  
	g_current_state = enable;
	PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, enable); // set output
}

/***************************************************************************//**
 * @brief Set the frequency of the buzzer
 * @param period 
 ******************************************************************************/
void
buzzer_per_set(uint16_t period){
	bool last_state = g_current_state;
	if(period < 3)
		period = 3;
	g_current_per = period;
	buzzer_write(false);
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, period);
	buzzer_vol_set(g_current_vol);
	buzzer_write(last_state);
}

/***************************************************************************//**
 * @brief Set the duty cycle value of the buzzer 
 * @param to do that, use the volume to set different levels
 ******************************************************************************/
void
buzzer_vol_set(uint16_t volume){
	uint16_t pulse_width;
	uint32_t period;
	g_current_vol = volume;
	
	// Gets the period of a PWM generator block
	period = PWMGenPeriodGet(PWM0_BASE, PWM_GEN_0);
	
	pulse_width = volume*period/0xFFFF;
	if(pulse_width >= period) pulse_width--; 
	if(pulse_width == 0) pulse_width++;
	
	// Sets the pulse width for the specified PWM output
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, pulse_width);
}

/***************************************************************************//**
 * @brief Initialize the peripherals, with the ports, pin and using the
 * information acquired on the Tive TM4C1294NCPDT Microcontroller datasheet
 * on the PWM Module Diagram.
 * Also, sets an initial value to set the duty cicle (volume) and the frequency.
 ******************************************************************************/
void 
buzzer_init(){
	g_ui32SysClock = 120000000; // 120MHz
	
	// Enable the PWM0 peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));
	
	// Wait for the PWM0 module to be ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
  // Configure PIN for use by the PWM peripheral
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

  buzzer_div_set(FREQ_0);
	
	// Configures the alternate function of a GPIO pin
	// PF1_M0PWM1 --> piezo buzzer
	GPIOPinConfigure(GPIO_PF1_M0PWM1);

  // Configures a PWM generator.
	// This function is used to set the mode of operation for a PWM generator.  The counting mode,
	// synchronization mode, and debug behavior are all configured. After configuration, the generator is left in the disabled state.
	PWMGenConfigure(PWM0_BASE, PWM_GEN_0, (PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC)); //PWM_GEN_0

	// Set the PWM period to 500Hz.  To calculate the appropriate parameter
	// use the following equation: N = (1 / f) * SysClk.  Where N is the
	// function parameter, f is the desired frequency, and SysClk is the
	// system clock frequency.
	// In this case you get: (1 / 500) * 120MHz / 64 = 3750 cycles.  Note that
	// the maximum period you can set is 2^16.
	// TODO: modify this calculation to use the clock frequency that you are
	// using.
	
	buzzer_per_set(0xFFFF);
	buzzer_vol_set(0x7FFF); 
	buzzer_write(false);
	
	PWMOutputInvert(PWM0_BASE, PWM_OUT_1_BIT, false);
	PWMGenEnable(PWM0_BASE, PWM_GEN_0);
}