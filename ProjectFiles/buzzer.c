#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
//#include "driverlib/rom.h"
#include "driverlib/pwm.h"

#include "buzzer.h"

//*****************************************************************************
// Piezo Buzzer
// pin40 - buzzer out - PF1 - M0PWM1 -- PWM out
//
// fazer funcao de alterar frequencia para mudar som
//*****************************************************************************

// MARI
// pin6  microphone - PE5 - AIN8

// JESSICA
// acelerometro
// x - pin23 - PE0 - AIN3
// y - pin24 - PE1 - AIN2
// z - pin25 - PE2 - AIN1
// joystick
// x - pin2  - PE4 - in  -- AIN9
// y - pin26 - PE3 - in  -- AIN0
// ! - pin5  - PC6 - gpio
//buttons
//b1 - pin33 - PL1 - gpio 
//b2 - pin32 - PL2 - gpio  

#define PWM_FREQUENCY			500

static uint32_t ui32SysClock;

void 
buzzer_init(){
	ui32SysClock = 120000000; // 120MHz
	
	// Enable the PWM0 peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));
	
	// Wait for the PWM0 module to be ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

  // Configure PIN for use by the PWM peripheral
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

	// Sets the PWM clock configuration
	// if you change DIV_64 to another value, it changes the frequency
	PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_64);
	
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
	
	//Sets the period of a PWM generator.
	// the period of the generator block is defined as the number of PWM clock ticks between pulses on the generator block zero signal.
	// if you change the 3750, it will change the frequency
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 3750); //PWM_GEN_0    so verificar freq e tal

	// Enables or disables PWM outputs
	// PWMOutputStat(uint32_t ui32Base, uint32_t ui32PWMOutBits, bool bEnable)
	//If bEnable is true, then the selected PWM outputs are enabled, or placed in the active state. If bEnable is false, then the selected outputs are disabled or placed in the inactive state.
	PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);

	// Sets the pulse width for the specified PWM output
	// This  function  sets  the  pulse  width  for  the  specified  PWM  output,  where  the  pulse  width  is defined as the number of PWM clock ticks
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 3500);

	// Enables the timer/counter for a PWM generator block
	//PWMGenEnable(PWM0_BASE, PWM_GEN_0);
	
}