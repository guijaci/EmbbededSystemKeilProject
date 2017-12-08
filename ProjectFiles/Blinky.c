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
#include "math.h"

#include "stdbool.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "grlib/grlib.h"

#include "rgb.h"
#include "cfaf128x128x16.h"
#include "servo.h"
#include "temp.h"
#include "opt.h"

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

static long double S01 = 0;

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
    rgb_write_color(ApplyLightness(RGB_YELLOW, .1));
    //Switch_On (LED_A);
    signal_func(tid_phaseB);                /* call common signal function   */
    //Switch_Off(LED_A);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
void phaseB (void const *argument) {
  for (;;) {
		osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    rgb_write_color(ApplyLightness(RGB_MAGENTA, .1));
    //Switch_On (LED_B);
    signal_func(tid_phaseC);                /* call common signal function   */
    //Switch_Off(LED_B);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
void phaseC (void const *argument) {
  for (;;) {
		osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    rgb_write_color(ApplyLightness (RGB_CYAN, .1));
    //Switch_On (LED_C);
    signal_func(tid_phaseD);                /* call common signal function   */
    //Switch_Off(LED_C);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
void phaseD (void  const *argument) {
  for (;;) {
		osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    rgb_write_color(RGB_OFF);
    //Switch_On (LED_D);
    signal_func(tid_phaseA);                /* call common signal function   */
    //Switch_Off(LED_D);
  }
}

static void intToString(int64_t value, char * pBuf, uint32_t len, uint32_t base){
    static const char* pAscii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int pos = 0;
    int64_t tmpValue = value;

    // the buffer must not be null and at least have a length of 2 to handle one
    // digit and null-terminator
    if (pBuf == NULL || len < 2)
        return;

    // a valid base cannot be less than 2 or larger than 36
    // a base value of 2 means binary representation. A value of 1 would mean only zeros
    // a base larger than 36 can only be used if a larger alphabet were used.
    if (base < 2 || base > 36)
        return;

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
        // the len parameter is invalid.
        return;

    pBuf[pos] = '\0';

    do {
        pBuf[--pos] = pAscii[value % base];
        value /= base;
    } while(value > 0);
}
int16_t TMP006_getTemp2( int16_t Vobj, int16_t Tdie )
{

		volatile long double Tobj=0.0;
		volatile long double fObj =0.0;
		long double Vos=0.0;
		long double S=0.0;
		long double a1;
    long double a2 ;
    long double b0;
    long double b1;
    long double b2 ;
    long double c2;
    long double Tref ;
	  long double Vobj2;
    long double Tdie2;
		int16_t k;
		int t;


   

    Tdie = Tdie >> 2;

    /* Calculate TMP006. This needs to be reviewed and calibrated */
    Vobj2 = (double)Vobj*.00000015625;
    Tdie2 = (double)Tdie*.03525 + 273.15;

    /* Initialize constants */
    S01 = 6 * pow(10, -14);
    a1 = 1.75*pow(10, -3);
    a2 = -1.678*pow(10, -5);
    b0 = -2.94*pow(10, -5);
    b1 = -5.7*pow(10, -7);
    b2 = 4.63*pow(10, -9);
    c2 = 13.4;
    Tref = 298.15;

    /* Calculate values */
    S = S01*(1+a1*(Tdie2 - Tref)+a2*pow((Tdie2 - Tref),2));
		Vos = b0 + b1*(Tdie2 - Tref) + b2*pow((Tdie2 - Tref),2);	
		fObj = (Vobj2 - Vos) + c2*pow((Vobj2 - Vos),2);
    Tobj = pow(pow(Tdie2,4) + (fObj/S),.25);
		k = Tobj - 273;
    /* Return temperature of object */
    return k;
}


/*----------------------------------------------------------------------------
 *      Thread 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
void clock (void  const *argument) {
	tContext sContext;
	tRectangle sRect;
	int16_t temp,temp2;
	long double teste;
	char buf[10];
	bool trig = true;
	int cont=0;
	
	GrContextInit(&sContext, &g_sCfaf128x128x16);

	sRect.i16XMin = 0;
	sRect.i16YMin = 0;
	sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
	sRect.i16YMax = 50;
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	temp = temp_read();
  for (;;) {
    osSignalWait(0x0100, osWaitForever);    /* wait for an event flag 0x0100 */
		temp = opt_read();
		intToString((uint16_t)temp, buf, 10, 10);
		GrContextForegroundSet(&sContext, ClrDarkBlue);
		GrRectFill(&sContext, &sRect);
		GrContextForegroundSet(&sContext, ClrWhite);
		GrStringDrawCentered(&sContext, buf, -1,
												 GrContextDpyWidthGet(&sContext) / 2, 10, 0);
		temp2 = OPT3001_getLux();
		intToString((uint16_t)temp2, buf, 10, 10);
		GrStringDrawCentered(&sContext, buf, -1,
												 GrContextDpyWidthGet(&sContext) / 2, 25, 0);
//		osDelay(100);  
//		temp2 = temp_read_temp();
//		intToString(temp2, buf, 10, 10);
//		GrStringDrawCentered(&sContext, buf, -1,
//												 GrContextDpyWidthGet(&sContext) / 2, 35, 0);
		GrFlush(&sContext);
    osDelay(5000);  
		cont--;/* delay 80ms                    */
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
	int16_t angle = 0;
	int16_t inc = 1;
	osKernelInitialize();
	cfaf128x128x16Init();
	rgb_init();
	servo_init();
	temp_init();
	opt_init();
	
  tid_phaseA = osThreadCreate(osThread(phaseA), NULL);
  tid_phaseB = osThreadCreate(osThread(phaseB), NULL);
  tid_phaseC = osThreadCreate(osThread(phaseC), NULL);
  tid_phaseD = osThreadCreate(osThread(phaseD), NULL);
  tid_clock  = osThreadCreate(osThread(clock),  NULL);
	
	osKernelStart();
	
	osSignalSet(tid_phaseA, 0x0001);          /* set signal to phaseA thread   */
	
	while(true){
		servo_write_degree180(angle);
		angle += inc;
		if(angle == 90 || angle == -90) 
			inc = -inc;	
		osDelay(100);
	}
  osDelay(osWaitForever);
  while(1);
}
