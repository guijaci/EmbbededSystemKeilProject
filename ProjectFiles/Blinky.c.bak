/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    BLinky.c
 *      Purpose: RTX example program
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2014 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include "LED.h"

#include "stdbool.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "grlib/grlib.h"
#include "driverlib/pin_map.h"
#include "cfaf128x128x16.h"
#include "driverlib/i2c.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"

osThreadId tid_phaseA;                  /* Thread id of thread: phase_a      */
osThreadId tid_phaseB;                  /* Thread id of thread: phase_b      */
osThreadId tid_phaseC;                  /* Thread id of thread: phase_c      */
osThreadId tid_phaseD;                  /* Thread id of thread: phase_d      */
osThreadId tid_clock;                   /* Thread id of thread: clock        */

#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    7

//Buffer para impressão de strings no LCD
char buf [10] ;
char buf1 [10] ;
//Converte um numero em uma base quanlquer para uma string de caracteres imprimiveis

static void intToString(int64_t value, char * pBuf, uint32_t len, uint32_t base){
    static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
    int pos = 0;
    int64_t tmpValue = value;

    // the buffer must not be null and at least have a length of 2 to handle one
    // digit and null-terminator
    if (pBuf == NULL || len < 2)
    {
        return;
    }

    // a valid base cannot be less than 2 or larger than 36
    // a base value of 2 means binary representation. A value of 1 would mean only zeros
    // a base larger than 36 can only be used if a larger alphabet were used.
    if (base < 2 || base > 36)
    {
        return;
    }

    // negative value
    if (value < 0)
    {
        tmpValue = -tmpValue;
        value    = -value;
        pBuf[pos++] = '-';
    }

    // calculate the required length of the buffer
    do {
        pos++;
        tmpValue /= base;
    } while(tmpValue > 0);


    if (pos > len)
    {
        // the len parameter is invalid.
        return;
    }

    pBuf[pos] = '\0';

    do {
        pBuf[--pos] = pAscii[value % base];
        value /= base;
    } while(value > 0);

    return;
}

void initI2C0(void)
{
   SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

   //reset I2C module
   SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

   //enable GPIO peripheral that contains I2C
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

   // Configure the pin muxing for I2C0 functions on port B2 and B3.
   GPIOPinConfigure(GPIO_PB2_I2C0SCL);
   GPIOPinConfigure(GPIO_PB3_I2C0SDA);

   // Select the I2C function for these pins.
   GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
   GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

   // Enable and initialize the I2C0 master module.  Use the system clock for
   // the I2C0 module.  The last parameter sets the I2C data transfer rate.
   // If false the data rate is set to 100kbps and if true the data rate will
   // be set to 400kbps.
   I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

}

uint8_t readI2C0(uint16_t device_address, uint16_t device_register)
{
	//specify that we want to communicate to device address with an intended write to bus
	I2CMasterSlaveAddrSet(I2C0_BASE, device_address, false);

	//the register to be read
	I2CMasterDataPut(I2C0_BASE, device_register);

	//send control byte and register address byte to slave device
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);

	//wait for MCU to complete send transaction
	while(I2CMasterBusy(I2C0_BASE));

	//read from the specified slave device
	I2CMasterSlaveAddrSet(I2C0_BASE, device_address, true);

	//send control byte and read from the register from the MCU
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

	//wait while checking for MCU to complete the transaction
	while(I2CMasterBusy(I2C0_BASE));

	//Get the data from the MCU register and return to caller
	return( I2CMasterDataGet(I2C0_BASE));
}

/*----------------------------------------------------------------------------
 *      Switch LED on
 *---------------------------------------------------------------------------*/
void Switch_On (unsigned char led) {

  if (led != LED_CLK) LED_On (led);
}

/*----------------------------------------------------------------------------
 *      Switch LED off
 *---------------------------------------------------------------------------*/
void Switch_Off (unsigned char led) {

  if (led != LED_CLK) LED_Off (led);
}


/*----------------------------------------------------------------------------
 *      Function 'signal_func' called from multiple threads
 *---------------------------------------------------------------------------*/
void signal_func (osThreadId tid)  {
  osSignalSet(tid_clock, 0x0100);           /* set signal to clock thread    */
  osDelay(500);                             /* delay 500ms                   */
  osSignalSet(tid_clock, 0x0100);           /* set signal to clock thread    */
  osDelay(500);                             /* delay 500ms                   */
  osSignalSet(tid, 0x0001);                 /* set signal to thread 'thread' */
  osDelay(500);                             /* delay 500ms                   */
}

/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
void phaseA (void const *argument) {
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_A);
    signal_func(tid_phaseB);                /* call common signal function   */
    Switch_Off(LED_A);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
void phaseB (void const *argument) {
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_B);
    signal_func(tid_phaseC);                /* call common signal function   */
    Switch_Off(LED_B);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
void phaseC (void const *argument) {
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_C);
    signal_func(tid_phaseD);                /* call common signal function   */
    Switch_Off(LED_C);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
void phaseD (void  const *argument) {
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_D);
    signal_func(tid_phaseA);                /* call common signal function   */
    Switch_Off(LED_D);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
void clock (void  const *argument) {
  for (;;) {
    osSignalWait(0x0100, osWaitForever);    /* wait for an event flag 0x0100 */
    Switch_On (LED_CLK);
    osDelay(80);                            /* delay 80ms                    */
    Switch_Off(LED_CLK);
  }
}

osThreadDef(phaseA, osPriorityNormal, 1, 0);
osThreadDef(phaseB, osPriorityNormal, 1, 0);
osThreadDef(phaseC, osPriorityNormal, 1, 0);
osThreadDef(phaseD, osPriorityNormal, 1, 0);
osThreadDef(clock,  osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
	
	
	tContext sContext;
	tRectangle sRect;
	
//	ROM_SysCtlDeepSleepClockConfigSet(16, SYSCTL_DSLP_OSC_INT);
//	SysCtlDeepSleepPowerSet(0x121);  // TSPD, FLASHPM = LOW_POWER_MODE, SRAMPM = STANDBY_MODE
	uint8_t teste , teste1;
	osKernelInitialize();
	
  SystemCoreClockUpdate();
  LED_Initialize();                         /* Initialize the LEDs           */
 	cfaf128x128x16Init();
	
	
	initI2C0();
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	GrContextFontSet(&sContext, g_psFontCm12);
	while(1){
	
	teste = readI2C0(0x40,0x00);
	teste1 = readI2C0(0x40,0x01);
	intToString(teste, buf, 10, 10);
	intToString(teste1, buf1, 10, 10);
	
//  tid_phaseA = osThreadCreate(osThread(phaseA), NULL);
//  tid_phaseB = osThreadCreate(osThread(phaseB), NULL);
//  tid_phaseC = osThreadCreate(osThread(phaseC), NULL);
//  tid_phaseD = osThreadCreate(osThread(phaseD), NULL);
//  tid_clock  = osThreadCreate(osThread(clock),  NULL);

	//osKernelStart();
	
	sRect.i16XMin = 0;
	sRect.i16YMin = 0;
	sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
	sRect.i16YMax = 36;
		
	GrContextForegroundSet(&sContext, ClrDarkBlue);
	GrRectFill(&sContext, &sRect);
		
	GrContextForegroundSet(&sContext, ClrWhite);
	GrRectDraw(&sContext, &sRect);
	
	GrStringDrawCentered(&sContext, buf, -1,
											 GrContextDpyWidthGet(&sContext) / 2, 10, 0);
	GrStringDrawCentered(&sContext, buf, -1,
											 GrContextDpyWidthGet(&sContext) / 2, 24, 0);
	GrFlush(&sContext);
	
	SysCtlDelay(65535);
	SysCtlDelay(65535);
	SysCtlDelay(65535);
}
	
//	osSignalSet(tid_phaseA, 0x0001);          /* set signal to phaseA thread   */
//	
//  osDelay(osWaitForever);
//  while(1);
}
