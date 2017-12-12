//..............................................................................
// adc.c - Driver for interfacing Kionix KXTC9-2050 accelerometer.
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
#include <stddef.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "grlib/grlib.h"
#include "accel.h"

/***************************************************************************//**
 * @brief Initialize ADC0 and the peripheral GPIO E 
 ******************************************************************************/
void accel_init(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
}

/***************************************************************************//**
 * @brief Read the analog value from ADC0 and return a uint16_t(voltage value).
 * @param  port is the port used to access the desired pin.
 * @param  pin to be read through ADC channel.
 * @param  channel to be read to get the value from the pin.
 * @return result[0] the voltage value corresponded to the channel.
 ******************************************************************************/
static uint16_t analog_read(uint32_t port, uint8_t pin, uint32_t channel)
{
	uint32_t result[1];

	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, channel | ADC_CTL_IE | ADC_CTL_END);
	GPIOPinTypeADC(port, pin);
	ADCSequenceConfigure(ADC0_BASE, 3,  ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceEnable(ADC0_BASE, 3);
	ADCIntClear(ADC0_BASE, 3);

	// Trigger the ADC conversion.	
	ADCProcessorTrigger(ADC0_BASE, 3);

	// Wait for conversion to be completed.
	while(!ADCIntStatus(ADC0_BASE, 3, false));
		
	// Clear the ADC interrupt flag.
	ADCIntClear(ADC0_BASE, 3);

	// Read ADC Value.
	ADCSequenceDataGet(ADC0_BASE, 3,  result);
	//ADCSequenceDisable(ADC0_BASE, 3);
 
	return result[0];
}

/***************************************************************************//**
 * @brief Read the x value from the accelerometer. 
 * @return it is the corresponded voltage value from the axis x
 ******************************************************************************/
uint16_t accel_read_x(void){
	return analog_read(GPIO_PORTE_BASE, GPIO_PIN_0, ADC_CTL_CH3);			
}

/***************************************************************************//**
 * @brief Read the y value from the accelerometer. 
 * @return it is the corresponded voltage value from the axis y
 ******************************************************************************/
uint16_t accel_read_y(void){
	return analog_read(GPIO_PORTE_BASE, GPIO_PIN_1, ADC_CTL_CH2);
}

/***************************************************************************//**
 * @brief Read the z value from the accelerometer. 
 * @return it is the corresponded voltage value from the axis z
 ******************************************************************************/
uint16_t accel_read_z(void){
	return analog_read(GPIO_PORTE_BASE, GPIO_PIN_2, ADC_CTL_CH1);
}