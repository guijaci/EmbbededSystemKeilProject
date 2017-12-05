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

#ifndef __SysCtlClockGet
#define __SysCtlClockGet()	\
SysCtlClockFreqSet( 				\
	SYSCTL_XTAL_25MHZ	| 			\
	SYSCTL_OSC_MAIN 	| 			\
	SYSCTL_USE_PLL 		| 			\
	SYSCTL_CFG_VCO_480, 			\
	120000000)
#endif

//Acionamento por PPM
//-Período Total: 20ms
//-Periodo Regime Maximo: 2ms
//-Periodo Regime Minimo: 1ms

//Formato de Onda: Regime Máximo
//  ========== __________ ____...___ __________ __________
// |          |                                           |
// |<- 2 ms ->|                                           |
// |                                                      |
// |<----------------------- 20 ms ---------------------->|


//Formato de Onda: Regime Mínimo
//  =====______ __________ ___...____ __________ __________
// |     |                                                |
// |<1ms>|                                                |
// |                                                      |
// |<----------------------- 20 ms ---------------------->|

#define CLK_F 16000000
#define MAX_T 0.002

static uint32_t g_ui32SysClock;
static uint16_t g_ui8Period, g_ui16perMin;

void
servo_write(uint16_t angle){
	TimerMatchSet(TIMER3_BASE, TIMER_B, g_ui16perMin*angle/0xFFFF + g_ui16perMin);
}

void 
servo_init(){
	uint32_t duty_cycle;	
	
	//Configura/ Recupera Clock
	g_ui32SysClock = __SysCtlClockGet();
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3) &
				!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));

	GPIOPinConfigure(GPIO_PM3_T3CCP1);
	GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_3);

	//Frequencia 16MHz
	TimerClockSourceSet(TIMER3_BASE, TIMER_CLOCK_PIOSC);
			
	//Configura timer como par (A/B) e PWM
	TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);
	
	TimerControlLevel(TIMER3_BASE, TIMER_B, true);
	TimerUpdateMode(TIMER3_BASE, TIMER_B, TIMER_UP_MATCH_TIMEOUT);

	//O periodo máximo para o timer PWM sem prescaler Tm é 0xFFFF / 16MHz ~= 4ms
	//Para chegar em 20ms, multiplicamos esse valor por cinco (cada unidade no prescale p 
	//aumenta em 4ms o Tm, portanto: p = 4 -> 20ms). No entanto, o duty cycle 
	//máximo DCm reduzira proporcionalmente ao p, pois o período em alta máximo Hm é fixo em 4ms:
	//  p		Hm/Tm [ms]		DCm [%]
	//	0	->	4/4  		->	100
	//  1	->	4/8  		->	 50
	//  2	->	4/12 		->	 33
	//  3	->	4/16		->	 25
	//  4	->	4/20 		->	 20
	//Como no PPM o duty cycle máximo é 10%, isto p = 4 é suficiente
	TimerPrescaleSet(TIMER3_BASE, TIMER_B, 4);

	//Período 2ms
	g_ui8Period = (uint16_t) CLK_F*MAX_T;
	//Periodo minimo 1ms = 2/2ms
	g_ui16perMin = g_ui8Period>>1;
	duty_cycle = g_ui16perMin;
		
	TimerLoadSet(TIMER3_BASE, TIMER_B, g_ui8Period);
	TimerMatchSet(TIMER3_BASE, TIMER_B, duty_cycle);
	TimerEnable(TIMER3_BASE, TIMER_B);
}
