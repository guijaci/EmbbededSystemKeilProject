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

#include "rgb.h"
#include "cfaf128x128x16.h"
#include "servo.h"

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
    rgb_write_color(RGB_RED);
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
    rgb_write_color(RGB_GREEN);
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
    rgb_write_color(RGB_BLUE);
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

/*----------------------------------------------------------------------------
 *      Thread 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
void clock (void  const *argument) {
  for (;;) {
    osSignalWait(0x0100, osWaitForever);    /* wait for an event flag 0x0100 */
    //Switch_On (LED_CLK);
    osDelay(80);                            /* delay 80ms                    */
    //Switch_Off(LED_CLK);
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
	
	osKernelInitialize();
	cfaf128x128x16Init();
	rgb_init();
	servo_init();
	
	GrContextInit(&sContext, &g_sCfaf128x128x16);

	sRect.i16XMin = 0;
	sRect.i16YMin = 0;
	sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
	sRect.i16YMax = 23;
	GrContextForegroundSet(&sContext, ClrDarkBlue);
	GrRectFill(&sContext, &sRect);

	GrContextForegroundSet(&sContext, ClrWhite);
	GrRectDraw(&sContext, &sRect);

	GrContextFontSet(&sContext, g_psFontCm12);
	GrStringDrawCentered(&sContext, "hello", -1,
											 GrContextDpyWidthGet(&sContext) / 2, 10, 0);

	GrContextFontSet(&sContext, g_psFontCm12/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext, "Hello World!", -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext) - 24) / 2) + 24,
											 0);

	GrFlush(&sContext);
	
  tid_phaseA = osThreadCreate(osThread(phaseA), NULL);
  tid_phaseB = osThreadCreate(osThread(phaseB), NULL);
  tid_phaseC = osThreadCreate(osThread(phaseC), NULL);
  tid_phaseD = osThreadCreate(osThread(phaseD), NULL);
  tid_clock  = osThreadCreate(osThread(clock),  NULL);
	
	osKernelStart();
	
	osSignalSet(tid_phaseA, 0x0001);          /* set signal to phaseA thread   */
	
  osDelay(osWaitForever);
  while(1);
}