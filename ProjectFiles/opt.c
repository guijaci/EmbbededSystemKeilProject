

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"

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
#define OPT3001_MANID 	0xFE
#define OPT3001_DEVID 	0x7F

#define OPT3001_CONFIG 	0x01

#define OPT3001_RESULT  0x00
#define OPT3001_LOW  		0x02
#define OPT3001_HIGH  	0x03

#define DEFAULT_CONFIG_100 0xC410
#define DEFAULT_CONFIG

#define I2C_WRITE false
#define I2C_READ 	true
	
static uint32_t g_ui32SysClock;
static uint16_t mid, did;

static void 
write16(uint8_t add, uint16_t data){
	uint8_t data_low  =  data & 0x00FF;
	uint8_t data_high = (data & 0xFF00)>>8;
	while(I2CMasterBusy(I2C0_BASE));
	I2CMasterSlaveAddrSet(I2C0_BASE, OPT3001_I2CADDR, I2C_WRITE);
	I2CMasterDataPut(I2C0_BASE, add);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
	I2CMasterDataPut(I2C0_BASE, data_high);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
	I2CMasterDataPut(I2C0_BASE, data_low);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

}

static uint16_t 
read16(uint8_t add){
	uint16_t data;
	while(I2CMasterBusy(I2C0_BASE));
	I2CMasterSlaveAddrSet(I2C0_BASE, OPT3001_I2CADDR, I2C_WRITE);
	I2CMasterDataPut(I2C0_BASE, add);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);	
	while(I2CMasterBusy(I2C0_BASE));
	
	
	I2CMasterBurstLengthSet(I2C0_BASE, 3);
	I2CMasterSlaveAddrSet(I2C0_BASE, OPT3001_I2CADDR, I2C_READ);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C0_BASE));
	data = (I2CMasterDataGet(I2C0_BASE));
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
	while(I2CMasterBusy(I2C0_BASE));
	data |= I2CMasterDataGet(I2C0_BASE) << 8;
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	return data;
}

int16_t opt_read(){
	return read16(OPT3001_RESULT);
}
void 
opt_init(){
	uint16_t temp = 0;
	g_ui32SysClock = __SysCtlClockGet();
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
	
	SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
	
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
	
	HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
	
	I2CMasterEnable(I2C0_BASE);
	
	write16(OPT3001_CONFIG, 0x01);
	SysCtlDelay(5000);

	mid = read16(OPT3001_MANID);
	did = read16(OPT3001_DEVID);
}

