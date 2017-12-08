#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"

#include "opt.h"

#ifndef __SysCtlClockGet
#define __SysCtlClockGet()	\
SysCtlClockFreqSet( 			\
	SYSCTL_XTAL_25MHZ	| 		\
	SYSCTL_OSC_MAIN 	| 		\
	SYSCTL_USE_PLL 		| 		\
	SYSCTL_CFG_VCO_480, 		\
	120000000)
#endif

#define OPT3001_I2CADDR 0x44
#define OPT3001_MANID 0xFE
#define OPT3001_DEVID 0x7F

#define OPT3001_CONFIG 0x01

#define OPT3001_RESULT  0x00
#define OPT3001_LOW  		0x02
#define OPT3001_HIGH  	0x03
#define OPT3001_RESET  	0x06
#define DEFAULT_CONFIG_100 0xC410 // 100ms

#define I2C_WRITE false
#define I2C_READ 	true
	
static uint32_t g_ui32SysClock;
static uint16_t mid, did;

#define I2C_MASTER_INT_DATA_NACK (I2C_MASTER_INT_NACK | I2C_MASTER_INT_DATA)
#define TIMEOUT16 0xFFFF

#define is_error(e) (e ? true : false)
#define start_stop_delay() 	SysCtlDelay(25)
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

static uint32_t 
send_single(uint8_t b){
	I2CMasterSlaveAddrSet(I2C0_BASE, OPT3001_I2CADDR, I2C_WRITE);
	I2CMasterDataPut(I2C0_BASE, b);
	while(I2CMasterBusBusy(I2C0_BASE));
	//Pelo menos 600ns
	start_stop_delay();	
	__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
	return I2CMasterErr(I2C0_BASE);
}

static uint32_t 
receive_single(uint8_t *b){
	uint32_t e;
	I2CMasterSlaveAddrSet(I2C0_BASE, OPT3001_I2CADDR, I2C_READ);
	while(I2CMasterBusBusy(I2C0_BASE));
	//Pelo menos 600ns
	start_stop_delay();	
	__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);	
	while(I2CMasterBusy(I2C0_BASE));
	e = I2CMasterErr(I2C0_BASE);
	if(is_error(e)) *b = I2CMasterDataGet(I2C0_BASE);
	return e;
}

static uint32_t
send_multiple(const uint8_t *b, uint8_t n){
	uint8_t i = 0;
	uint32_t e;
	if(n < 2) { 
		if(n == 1) 
			return send_single(*b);
		return I2C_MASTER_ERR_NONE;
	}
	
	I2CMasterSlaveAddrSet(I2C0_BASE, OPT3001_I2CADDR, I2C_WRITE);
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

static uint32_t 
receive_multiple(uint8_t *b, uint8_t n){
	uint8_t i = 0;
	uint32_t e;
	if(n < 2) { 
		if(n == 1) 
			return receive_single(b);
		return I2C_MASTER_ERR_NONE;
	}
	
	I2CMasterSlaveAddrSet(I2C0_BASE, OPT3001_I2CADDR, I2C_READ);
	I2CMasterBurstLengthSet(I2C0_BASE, n);
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
			while(I2CMasterBusy(I2C0_BASE));
			e = I2CMasterErr(I2C0_BASE);
			if(is_error(e)){
				//Pelo menos 600ns
				start_stop_delay();	
				__I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);	
				return e;
			}
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

static void 
write_reg(uint8_t add, uint16_t data){
	uint32_t bytes = add | (data & 0xFF00) | (data & 0x00FF)<<16;
	while(I2CMasterBusy(I2C0_BASE));
	send_multiple((uint8_t*)&bytes, 3);
}

static uint16_t 
read_reg(uint8_t add){
	uint16_t data;
	while(I2CMasterBusy(I2C0_BASE));
	send_single(add);
	start_stop_delay();
	receive_multiple((uint8_t*) &data, 2);
	return (data & 0x00FF)<<8 | (data & 0xFF00)>>8;
}

int16_t opt_read(){
	return read_reg(OPT3001_RESULT);
}

static void
temp_int_callback(void){
	I2CMasterIntClearEx(I2C0_BASE, I2C_MASTER_INT_DATA_NACK);
	I2CMasterIntClear(I2C0_BASE);
	g_sentFlag = true;
}

uint32_t OPT3001_getLux()
{
    uint16_t exponent = 0;
    uint32_t result = 0;
    int16_t raw;
    raw = read_reg(OPT3001_RESULT);
    /*Convert to LUX*/
    //extract result & exponent data from raw readings
    result = raw&0x0FFF;
    exponent = (raw>>12)&0x000F;
    //convert raw readings to LUX
    switch(exponent){
    case 0: //*0.015625
        result = result>>6;
        break;
    case 1: //*0.03125
        result = result>>5;
        break;
    case 2: //*0.0625
        result = result>>4;
        break;
    case 3: //*0.125
        result = result>>3;
        break;
    case 4: //*0.25
        result = result>>2;
        break;
    case 5: //*0.5
        result = result>>1;
        break;
    case 6:
        result = result;
        break;
    case 7: //*2
        result = result<<1;
        break;
    case 8: //*4
        result = result<<2;
        break;
    case 9: //*8
        result = result<<3;
        break;
    case 10: //*16
        result = result<<4;
        break;
    case 11: //*32
        result = result<<5;
        break;
    }
    return result;
}
void 
opt_init(){
		uint64_t temp = 0;
	g_ui32SysClock = __SysCtlClockGet();
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
	
	SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0) 	& 
				!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)	&
				!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP));
	
	GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_2);
	
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
	GPIOPinConfigure(GPIO_PB3_I2C0SDA);
	
	GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
	GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
	
	I2CMasterTimeoutSet(I2C0_BASE, 0xFFFFFFFF);
	I2CMasterGlitchFilterConfigSet(I2C0_BASE, I2C_MASTER_GLITCH_FILTER_8);
	I2CMasterInitExpClk(I2C0_BASE, g_ui32SysClock, false);
	
	I2CMasterIntEnableEx(I2C0_BASE, I2C_MASTER_INT_DATA_NACK);
	I2CIntRegister(I2C0_BASE, temp_int_callback);
	IntEnable(INT_I2C0_TM4C129);
	
	HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
	
	I2CMasterEnable(I2C0_BASE);

	write_reg(OPT3001_CONFIG, 0x00);
	SysCtlDelay(5000);
	write_reg(OPT3001_CONFIG, 0x06);

	write_reg(OPT3001_CONFIG, DEFAULT_CONFIG_100);
	mid = read_reg(OPT3001_MANID);
	did = read_reg(OPT3001_DEVID);
}
