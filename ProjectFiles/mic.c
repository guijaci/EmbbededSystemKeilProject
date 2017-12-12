//..............................................................................
// mic.c - Driver for the Microphone
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

// pin 6 - PE5 - AIN8 -- analog in

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

/***************************************************************************//**
 * @brief Initialize the microphone ADC0 and the peripheral GPIOE.
 ******************************************************************************/
void 
	mic_init(void){
	
	// Enable the ADC0 peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	// Enable the Port E peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
}

/***************************************************************************//**
 * @brief Read the analog value from ADC0 and return a uint16_t(voltage value).
 * @param  port is the port used to access the desired pin.
 * @param  pin to be read through ADC channel.
 * @param  channel to be read to get the value from the pin.
 * @return result[0] the voltage value corresponded to the channel.
 ******************************************************************************/
static 
	uint16_t analog_read(uint32_t port, uint8_t pin, uint32_t channel){
		uint32_t result[1];
		
		//Configure a step of the sample sequencer
		ADCSequenceStepConfigure(ADC0_BASE, 3, 0, channel | ADC_CTL_IE | ADC_CTL_END);
		
		//Configure pin for use as analog-to-digital converter inputs
		GPIOPinTypeADC(port, pin);
		
		// Configures the trigger source and priority of a sample sequence.
    ADCSequenceConfigure(ADC0_BASE, 3,  ADC_TRIGGER_PROCESSOR, 0);
		
		// Enables a sample sequence.
    ADCSequenceEnable(ADC0_BASE, 3);
		
		// Clears sample sequence interrupt source
    ADCIntClear(ADC0_BASE, 3);

		// Trigger the ADC conversion.
		ADCProcessorTrigger(ADC0_BASE, 3);


		// Wait for conversion to be completed.
		while(!ADCIntStatus(ADC0_BASE, 3, false));
		
		// Clear the ADC interrupt flag.
		ADCIntClear(ADC0_BASE, 3);

		// Read ADC Value.
		ADCSequenceDataGet(ADC0_BASE, 3,  result);
 
	return result[0];
}

/***************************************************************************//**
 * @brief Read the analog value from ADC0 and return a uint16_t(voltage value).
 * @return the voltage value corresponded to the channel.
 ******************************************************************************/
uint16_t
mic_read(void){
	return analog_read(GPIO_PORTE_BASE, GPIO_PIN_5, ADC_CTL_CH8);
}