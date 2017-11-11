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
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "stdbool.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"


osThreadId tid_taskA;                  /* Thread id of thread: task_a      */
osThreadId tid_taskB;                  /* Thread id of thread: task_b      */
osThreadId tid_taskC;                  /* Thread id of thread: task_c      */
osThreadId tid_taskD;                  /* Thread id of thread: task_d      */

osThreadId tid_taskE;                  /* Thread id of thread: task_e        */
osThreadId tid_taskF;                  /* Thread id of thread: task_e        */
osThreadId tid_drawer;                  /* Thread id of thread: drawer        */

#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    7
#define PI (3.141592653589793)

int i;
volatile int time1;

uint8_t buf [10];
char casa [10] ;
//====================================================
// nome
//====================================================
enum state{ready, running, waiting};

struct taskDetails{
   int8_t dynamic_Priority;
   int8_t static_Priority;
	 enum state	task_state;// if it is ready, running or waiting
	 uint8_t deadline; //in ticks
	 int time; //in ticks, time of execution
	 uint8_t execution_percentage; // Percentual de execução
	
};

struct taskDetails taskA_details;
struct taskDetails taskB_details;
struct taskDetails taskC_details;
struct taskDetails taskD_details;
struct taskDetails taskE_details;
struct taskDetails taskF_details;
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


//FATORIAL
unsigned long fatorial(unsigned long x)
{
    if ( x == 0 ) 
        return 1;
    return(x * fatorial(x - 1));
}

static void intToString(int value, char * pBuf, uint32_t len, uint32_t base){
    static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
    int pos = 0;
    int tmpValue = value;

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


/*----------------------------------------------------------------------------
 *      Function 'signal_func' called from multiple threads
 *---------------------------------------------------------------------------*/
//void signal_func (osThreadId tid)  {
//  osSignalSet(tid_clock, 0x0100);           /* set signal to clock thread    */
//  osDelay(500);                             /* delay 500ms                   */
//  osSignalSet(tid_clock, 0x0100);           /* set signal to clock thread    */
//  osDelay(500);                             /* delay 500ms                   */
//  osSignalSet(tid, 0x0001);                 /* set signal to thread 'thread' */
//  osDelay(500);                             /* delay 500ms                   */
//}

/*----------------------------------------------------------------------------
 *      Thread 1 'taskA': task A output
 *---------------------------------------------------------------------------*/
void taskA (void const *argument) {
	volatile int time;
  int32_t val;
	unsigned long a;
	int x;
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_A);
                
		 time = osKernelSysTick();
			for(x = 0 ; x<=256 ; x++)
			{
				a = (x+(x+2));
			}
		taskA_details.time = osKernelSysTick() - time;

		osSignalSet(tid_taskB, 0x0001);
    Switch_Off(LED_A);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 2 'taskB': task B output
 *---------------------------------------------------------------------------*/
void taskB (void const *argument) {
 uint32_t time;
 int32_t val;
 unsigned long b;
		int x;
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_B);
		osSignalSet(tid_taskC, 0x0001);
		time = osKernelSysTick();
		for( x = 1 ; x<=16 ; x++)
			{
				b = (2^x)/fatorial(x);
			}
    taskB_details.time = osKernelSysTick() - time;        /* call common signal function   */
    Switch_Off(LED_B);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 3 'taskC': task C output
 *---------------------------------------------------------------------------*/
void taskC (void const *argument) {
	volatile int time;
	int32_t val;
	unsigned long c;
		int x;
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_C);
		time = osKernelSysTick();
		
			for(x = 1 ; x<=72 ; x++)
			{
				c = (x+1)/x;
				
			}
			
			taskC_details.time = osKernelSysTick() - time;


		osSignalSet(tid_taskD, 0x0001);
    Switch_Off(LED_C);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 4 'taskD': task D output
 *---------------------------------------------------------------------------*/
void taskD (void  const *argument) {
	
	volatile int time;
  volatile int time1;
	int32_t val;
	unsigned long  d;
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_D);
		time = osKernelSysTick();
		d = 1 + (5/fatorial(3))+(5/fatorial(5))+ (5/fatorial(7))+(5/fatorial(9));
		taskD_details.time = osKernelSysTick() - time;;

    Switch_Off(LED_D);
		osSignalSet(tid_taskE, 0x0001);
  }
}

void taskE (void  const *argument) {
	int x;
	unsigned long e;
	volatile int time;
  volatile int time1;
	int32_t val;
	unsigned long  d;
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_D);
		time = osKernelSysTick();
			for(x = 1 ; x<=100 ;x++)
			{
				e = x*PI*PI;
			}
		taskE_details.time = osKernelSysTick() - time;;

   // Switch_Off(LED_D);
		osSignalSet(tid_drawer, 0x0001);
  }
}

void taskF(void  const *argument) {
	int x;
	volatile int time;
  volatile int time1;
	unsigned long f;
	int32_t val;
	unsigned long  d;
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
  //  Switch_On (LED_D);
		time = osKernelSysTick();
		for(x = 1 ; x<=128 ; x++)
			{
				f = (x*x*x)/(x*x);
			}
		taskF_details.time = osKernelSysTick() - time;

//    Switch_Off(LED_D);
		osSignalSet(tid_drawer, 0x0001);
  }
}
///*----------------------------------------------------------------------------
// *      Thread 5 'clock': Signal Clock
// *---------------------------------------------------------------------------*/
//void clock (void  const *argument) {
//  for (;;) {
//    osSignalWait(0x0100, osWaitForever);    /* wait for an event flag 0x0100 */
//    Switch_On (LED_CLK);
//    osDelay(80);                            /* delay 80ms                    */
//    Switch_Off(LED_CLK);
//  }
//}

void drawer () {

	tContext sContext;
	tRectangle sRect;
	GrContextInit(&sContext, &g_sCfaf128x128x16);
 
  while (1) {
	osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
	Switch_On (LED_CLK);
//	sRect.i16XMin = 0;
//	sRect.i16YMin = 0;
//	sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
//	sRect.i16YMax = 23;
//	GrContextForegroundSet(&sContext, ClrDarkBlue);
		GrRectFill(&sContext, &sRect);

	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrRectDraw(&sContext, &sRect);
	
	GrContextFontSet(&sContext, g_psFontCm12);
	intToString(33, casa, 10, 10);
//	GrStringDrawCentered(&sContext,(char*)casa, -1,
//											 GrContextDpyWidthGet(&sContext) / 2, 10, 0);
	GrFlush(&sContext);
	intToString(taskA_details.time, casa, 10, 10);
	GrContextFontSet(&sContext,g_psFontFixed6x8);
	GrStringDrawCentered(&sContext,(char*)casa, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 120)) + 10,
											 0);
	
	intToString(taskB_details.time, casa, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)casa, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 110)) + 10,
											 0);
	
	intToString(taskC_details.time, casa, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)casa, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 100)) + 10,
											 0);
	
	intToString(taskD_details.time, casa, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)casa, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 90)) + 10,0);
	
	
	intToString(taskE_details.time, casa, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)casa, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 80)) + 10,0);
	
	intToString(taskF_details.time, casa, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)casa, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 70)) + 10,0);

	GrFlush(&sContext);
	osDelay(5000);   
	Switch_Off(LED_CLK);
	osSignalSet(tid_taskA, 0x0001);
  }
}

osThreadDef(taskA, osPriorityNormal, 1, 0);
osThreadDef(taskB, osPriorityNormal, 1, 0);
osThreadDef(taskC, osPriorityNormal, 1, 0);
osThreadDef(taskD, osPriorityNormal, 1, 0);
osThreadDef(taskE, osPriorityNormal, 1, 0);
osThreadDef(taskF, osPriorityNormal, 1, 0);
osThreadDef(drawer,  osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
//	tcontext scontext;
//	trectangle srect;
	
//	ROM_SysCtlDeepSleepClockConfigSet(16, SYSCTL_DSLP_OSC_INT);
//	SysCtlDeepSleepPowerSet(0x121);  // TSPD, FLASHPM = LOW_POWER_MODE, SRAMPM = STANDBY_MODE

	osKernelInitialize();
	
  SystemCoreClockUpdate();
  LED_Initialize();                         /* Initialize the LEDs           */
 	cfaf128x128x16Init();
	
//	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
  tid_taskA = osThreadCreate(osThread(taskA), NULL);
  tid_taskB = osThreadCreate(osThread(taskB), NULL);
  tid_taskC = osThreadCreate(osThread(taskC), NULL);
  tid_taskD = osThreadCreate(osThread(taskD), NULL);
	tid_taskE = osThreadCreate(osThread(taskE), NULL);
	tid_taskF = osThreadCreate(osThread(taskF), NULL);
	tid_drawer = osThreadCreate(osThread(drawer),  NULL);

	osKernelStart();
	
	
	//************************
  //Thread Main
  //************************
  tid_drawer = osThreadGetId();
  osThreadSetPriority(tid_drawer, osPriorityBelowNormal);
  drawer();
	
//	sRect.i16XMin = 0;
//	sRect.i16YMin = 0;
//	sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
//	sRect.i16YMax = 23;
//	GrContextForegroundSet(&sContext, ClrDarkBlue);
//	GrRectFill(&sContext, &sRect);

//	intToString(33, casa, 10, 10);
//	GrContextForegroundSet(&sContext, ClrWhite);
//	GrRectDraw(&sContext, &sRect);
//	
//	GrContextFontSet(&sContext, g_psFontCm12);
//	GrStringDrawCentered(&sContext,(char*)casa, -1,
//											 GrContextDpyWidthGet(&sContext) / 2, 10, 0);

//	GrContextFontSet(&sContext, g_psFontCm12/*g_psFontFixed6x8*/);
//	GrStringDrawCentered(&sContext, "Hello World!", -1,
//											 GrContextDpyWidthGet(&sContext) / 2,
//											 ((GrContextDpyHeightGet(&sContext) - 24) / 2) + 24,
//											 0);

//	GrFlush(&sContext);
	
	osSignalSet(tid_taskA, 0x0001);          /* set signal to taskA thread   */
	
  osDelay(osWaitForever);
  while(1);
}
