//

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "servo.h"

static uint32_t ui32SysClock;

void 
servo_init(){
	uint32_t ulPeriod;
	uint32_t dutyCycle, min;	
	
	ui32SysClock = 120000000;
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3));
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));

	GPIOPinConfigure(GPIO_PM3_T3CCP1);
	GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_3);

	TimerClockSourceSet(TIMER3_BASE, TIMER_CLOCK_SYSTEM);
			
	TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);
	
	TimerControlLevel(TIMER3_BASE, TIMER_B, true);
	TimerUpdateMode(TIMER3_BASE, TIMER_B, TIMER_UP_MATCH_TIMEOUT);

	TimerPrescaleSet(TIMER3_BASE, TIMER_B, 4);

	ulPeriod = 65535;
	min = ulPeriod/2;
	dutyCycle = min;
		
	TimerLoadSet(TIMER3_BASE, TIMER_B, ulPeriod - 1);
	TimerMatchSet(TIMER3_BASE, TIMER_B, dutyCycle);
	TimerEnable(TIMER3_BASE, TIMER_B);
	
}
