
//..............................................................................
// temp.c - Driver for interfacing TMP006.
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
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"

#include "temp.h"

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

#define TMP006_CONFIG     0x02

#define TMP006_CFG_RESET    0x8000
#define TMP006_CFG_MODEON   0x7000
#define TMP006_CFG_1SAMPLE  0x0000
#define TMP006_CFG_2SAMPLE  0x0200
#define TMP006_CFG_4SAMPLE  0x0400
#define TMP006_CFG_8SAMPLE  0x0600
#define TMP006_CFG_16SAMPLE 0x0800
#define TMP006_CFG_DRDYEN   0x0100
#define TMP006_CFG_DRDY     0x0080

#define TMP006_I2CADDR 0x40
#define TMP006_MANID 0xFE
#define TMP006_DEVID 0xFF

#define TMP006_VOBJ  0x0
#define TMP006_TAMB 0x01

#define I2C_WRITE false
#define I2C_READ 	true

/*******************************************************************************
 * @brief Enable calkback interruption
 *******************************************************************************/
#define I2C_MASTER_INT_DATA_NACK (I2C_MASTER_INT_NACK | I2C_MASTER_INT_DATA)
#define TIMEOUT16 0xFFFF

#define is_error(e) (e ? true : false)
	
/*******************************************************************************
 * @brief delay 600ns
 *******************************************************************************/
#define start_stop_delay() 	SysCtlDelay(25)

/*******************************************************************************
 * @brief Redefining this macro overwrite the default method for getting system 
 *				clock. Tiva TM4C1294 systems can't use the TivaWare's SysCtlClockGet() 
 *				to get the system clock, so SysCtlClockFreqSet() is used instead.
 *******************************************************************************/
#define __I2CMasterControl(base, command) 	\
	g_sentFlag = false;												\
	I2CMasterControl(base, command);					\
	g_timeout_count = 0;											\
	while(!g_sentFlag) 												\
		if(g_timeout_count++ >= TIMEOUT16) 			\
			break;

static uint32_t g_ui32SysClock;
static uint16_t g_mid, g_did, g_timeout_count;
static bool g_sentFlag;

		
/***************************************************************************//**
 * @brief  Writes byte data to the sensor
 * @param  b Address of register you want to modify
 * @return uint32_t  error if had
 ******************************************************************************/
static uint32_t 
send_single(uint8_t b){
	I2CMasterSlaveAddrSet(I2C0_BASE, TMP006_I2CADDR, I2C_WRITE);
	I2CMasterDataPut(I2C0_BASE, b);
	while(I2CMasterBusBusy(I2C0_BASE));
	//Pelo menos 600ns
	start_stop_delay();	
	__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
	return I2CMasterErr(I2C0_BASE);
}

/***************************************************************************//**
 * @brief  Reads a byte data from the sensor by I2C
 * @param  b Pointer wished to be recived the data 
 * @return uint32_t  error if had
 ******************************************************************************/
static uint32_t 
receive_single(uint8_t *b){
	uint32_t e;
	I2CMasterSlaveAddrSet(I2C0_BASE, TMP006_I2CADDR, I2C_READ);
	while(I2CMasterBusBusy(I2C0_BASE));
	//Pelo menos 600ns
	start_stop_delay();	
	__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);	
	while(I2CMasterBusy(I2C0_BASE));
	e = I2CMasterErr(I2C0_BASE);
	if(is_error(e)) *b = I2CMasterDataGet(I2C0_BASE);
	return e;
}

/***************************************************************************//**
 * @brief  Writes multiple bytes data to the sensor by i2C bus
 * @param  b Pointer wished to be written the data 
 * @param  n numbers of bytes
 * @return uint32_t  error if had
 ******************************************************************************/
static uint32_t
send_multiple(const uint8_t *b, uint8_t n){
	uint8_t i = 0;
	uint32_t e;
	if(n < 2) { 
		if(n == 1) 
			return send_single(*b);
		return I2C_MASTER_ERR_NONE;
	}
	
	I2CMasterSlaveAddrSet(I2C0_BASE, TMP006_I2CADDR, I2C_WRITE);
	I2CMasterBurstLengthSet(I2C0_BASE, n);
	I2CMasterDataPut(I2C0_BASE, b[i++]);
	while(I2CMasterBusBusy(I2C0_BASE));
	//Pelo menos 600ns
	start_stop_delay();	
	__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);	
	switch(n){
		while(i < n){
			//Pelo menos 600ns
			start_stop_delay();	
			__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);	
			default:
			while(I2CMasterBusy(I2C0_BASE));
			e = I2CMasterErr(I2C0_BASE);
			if(is_error(e)){
				//Pelo menos 600ns
				start_stop_delay();	
				__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);	
				return e;
			}
			I2CMasterDataPut(I2C0_BASE, b[i++]);
			start_stop_delay();
		}
	}
	//Pelo menos 600ns
	start_stop_delay();	
	__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);	
	while(I2CMasterBusy(I2C0_BASE));
	return I2CMasterErr(I2C0_BASE);
}

/***************************************************************************//**
 * @brief  Reads multiple bytes data to the sensor by i2C bus
 * @param  b Pointer wished to be recived the data 
 * @param  n numbers of bytes
 * @return uint32_t  error if had
 ******************************************************************************/
static uint32_t 
receive_multiple(uint8_t *b, uint8_t n){
	uint8_t i = 0;
	uint32_t e;
	if(n < 2) { 
		if(n == 1) 
			return receive_single(b);
		return I2C_MASTER_ERR_NONE;
	}
	//define master e slave e tipo de operação
	I2CMasterSlaveAddrSet(I2C0_BASE, TMP006_I2CADDR, I2C_READ);
	//define  tamanho numero de bytes a serem enviados
	I2CMasterBurstLengthSet(I2C0_BASE, n);
	//espera por envio de dados do barramento I2C
	while(I2CMasterBusBusy(I2C0_BASE));
	//Pelo menos 600ns
	start_stop_delay();	
	__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);	
	switch(n){
		while(i < n-1){
			//Pelo menos 600ns
			start_stop_delay();	
			__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);	
			default:
			//espera por envio de dados do barramento I2C
			while(I2CMasterBusy(I2C0_BASE));
			//verificaão de erro
			e = I2CMasterErr(I2C0_BASE);
			if(is_error(e)){
				//Pelo menos 600ns
				start_stop_delay();	
				__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);	
				return e;
			}
			//receve os dados
			b[i++] = I2CMasterDataGet(I2C0_BASE);
			//Pelo menos 600ns
			start_stop_delay();
		}
	}
	//Pelo menos 600ns
	start_stop_delay();
	__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);	
	while(I2CMasterBusy(I2C0_BASE));
	e = I2CMasterErr(I2C0_BASE);
	b[i] = I2CMasterDataGet(I2C0_BASE);
	return e;
}

/***************************************************************************//**
 * @brief  Choose the  the correct form
 * @param  add Address of register to read from
 * @param  data Data to be written to the specified register
 * @return none
 ******************************************************************************/
static void 
write_reg(uint8_t add, uint16_t data){
	uint32_t bytes = add | (data & 0xFF00) | (data & 0x00FF)<<16;
	while(I2CMasterBusy(I2C0_BASE));
	send_multiple((uint8_t*)&bytes, 3);
}
/***************************************************************************//**
 * @brief  Reads data from the sensor
 * @param  add Address of register to read from
 * @return Register contents
 ******************************************************************************/
static uint16_t 
read_reg(uint8_t add){
	uint16_t data;
	while(I2CMasterBusy(I2C0_BASE));
	send_single(add);
	start_stop_delay();
	receive_multiple((uint8_t*) &data, 2);
	return (data & 0x00FF)<<8 | (data & 0xFF00)>>8;
}


/***************************************************************************//**
 * @brief  Reads data temperature from the sensor  
 * @param  none
 * @return temperature in degrees celsius
 ******************************************************************************/
int16_t temp_read(void){
	return read_reg(TMP006_TAMB)>>2;
}

/***************************************************************************//**
 * @brief  Reads data voltage from the sensor 
 * @param  none
 * @return voltege
 ******************************************************************************/
int16_t temp_read_voltage(void){
	return read_reg(TMP006_VOBJ);
}

/***************************************************************************//**
 * @brief Clear interrption and alter g_sentFlag
 * @param  none
 * @return none
 ******************************************************************************/

static void
temp_int_callback(void){
	I2CMasterIntClearEx(I2C0_BASE, I2C_MASTER_INT_DATA_NACK);
	I2CMasterIntClear(I2C0_BASE);
	g_sentFlag = true;
}

/***************************************************************************//**
 * @brief  Configures the I@C and TMP006 Infrared Thermopile Sensor
 * @param  none
 * @return none
 ******************************************************************************/
void 
temp_init(){
	uint64_t temp = 0;
	g_ui32SysClock = __SysCtlClockGet();
	//habilita ports B e P e Barramento I2C
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
	
	SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0) 	& 
				!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)	&
				!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP));
	
	GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_2);
	
	//preaparando para inicializão dos pinos do I2C
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
	GPIOPinConfigure(GPIO_PB3_I2C0SDA);
	//inicializando pinos I2C
	GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
	GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
	
	I2CMasterTimeoutSet(I2C0_BASE, 0xFFFFFFFF);
	I2CMasterGlitchFilterConfigSet(I2C0_BASE, I2C_MASTER_GLITCH_FILTER_8);
	I2CMasterInitExpClk(I2C0_BASE, g_ui32SysClock, false);
	
	I2CMasterIntEnableEx(I2C0_BASE, I2C_MASTER_INT_DATA_NACK);
	I2CIntRegister(I2C0_BASE, temp_int_callback);
	IntEnable(INT_I2C0_TM4C129);
	// limpa o Buffer de dados
	HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
	
	I2CMasterEnable(I2C0_BASE);
	

	write_reg(TMP006_CONFIG, TMP006_CFG_RESET);
	write_reg(TMP006_CONFIG, TMP006_CFG_MODEON | TMP006_CFG_DRDYEN | TMP006_CFG_16SAMPLE);
	//write_reg(TMP006_CONFIG, TMP006_CFG_MODEON | TMP006_CFG_2SAMPLE);

	g_mid = read_reg(TMP006_MANID);
	g_did = read_reg(TMP006_DEVID);
}
