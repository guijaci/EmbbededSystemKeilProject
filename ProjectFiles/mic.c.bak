//..............................................................................
// mic.c - Driver for the Speaker.
//
// Copyright (c) 2017 Allan Patrick de Souza, Guilherme Jacichen, Jessica Isoton Sampaio,
// Mariana Carrião.  All rights reserved.
// Software License Agreement
//..............................................................................

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "grlib/grlib.h"

#include "mic.h"

void mic_init(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
}

static uint16_t analog_read(uint32_t port, uint8_t pin, uint32_t channel)
{
		uint32_t result[1];
	
		ADCSequenceStepConfigure(ADC0_BASE, 3, 0, channel | ADC_CTL_IE | ADC_CTL_END);
		GPIOPinTypeADC(port, pin);
    ADCSequenceConfigure(ADC0_BASE, 3,  ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);
		//
		// Trigger the ADC conversion.
		//
		ADCProcessorTrigger(ADC0_BASE, 3);

		//
		// Wait for conversion to be completed.
		//
		while(!ADCIntStatus(ADC0_BASE, 3, false));
		
		//
		// Clear the ADC interrupt flag.
		//
		ADCIntClear(ADC0_BASE, 3);

		//
		// Read ADC Value.
		//
		
		ADCSequenceDataGet(ADC0_BASE, 3,  result);
		//ADCSequenceDisable(ADC0_BASE, 3);
 
	return result[0];
}

uint16_t mic_read(void){
	return analog_read(GPIO_PORTE_BASE, GPIO_PIN_5, ADC_CTL_CH8);
}