//

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"

#include "temp.h"

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

static uint32_t g_ui32SysClock;
static uint16_t mid, did;

static void 
write16(uint8_t add, uint16_t data){
	uint8_t data_low  =  data & 0x00FF;
	uint8_t data_high = (data & 0xFF00)>>8;
	while(I2CMasterBusy(I2C0_BASE));
	I2CMasterSlaveAddrSet(I2C0_BASE, TMP006_I2CADDR, I2C_WRITE);
	I2CMasterDataPut(I2C0_BASE, add);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
	I2CMasterDataPut(I2C0_BASE, data_high);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
	I2CMasterDataPut(I2C0_BASE, data_low);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
}

static uint16_t 
read16(uint8_t add){
	uint16_t data;
	while(I2CMasterBusy(I2C0_BASE));
	I2CMasterSlaveAddrSet(I2C0_BASE, TMP006_I2CADDR, I2C_WRITE);
	I2CMasterDataPut(I2C0_BASE, add);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
	
	
	I2CMasterBurstLengthSet(I2C0_BASE, 2);
	I2CMasterSlaveAddrSet(I2C0_BASE, TMP006_I2CADDR, I2C_READ);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C0_BASE));
	data = (I2CMasterDataGet(I2C0_BASE));
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
	while(I2CMasterBusy(I2C0_BASE));
	data |= I2CMasterDataGet(I2C0_BASE) << 8;
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

	return (uint16_t)(data & 0xFFFF);
}

int16_t temp_read(){
	return read16(TMP006_MANID);
}

void 
temp_init(){
	uint16_t temp = 0;
	g_ui32SysClock = __SysCtlClockGet();
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0) 	& 
				!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)	&
				!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP));
	
	//GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_2);
	
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
	GPIOPinConfigure(GPIO_PB3_I2C0SDA);
	
	GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
	GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
	
	//I2CMasterTimeoutSet(I2C0_BASE, 0x09FF);
	//I2CMasterGlitchFilterConfigSet
	I2CMasterInitExpClk(I2C0_BASE, g_ui32SysClock, false);
	
	I2CMasterEnable(I2C0_BASE);
	
	write16(TMP006_CONFIG, TMP006_CFG_RESET);
	SysCtlDelay(5000);
	write16(TMP006_CONFIG, TMP006_CFG_MODEON | TMP006_CFG_DRDYEN | TMP006_CFG_8SAMPLE);
	SysCtlDelay(5000);

	mid = read16(TMP006_MANID);
	did = read16(TMP006_DEVID);
}