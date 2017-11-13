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
osThreadId tid_scheduler;
#define LED_0      0
#define LED_1      1
#define LED_2      2
#define LED_3      3
#define LED_CLK    7
#define PI (3.141592653589793)
#define PI2 PI*PI
int i;
volatile int time1;

char buf [10] ;


//Timer
void timer_callback(const void* args);
static osTimerId timerScheduler;                             // timer id
static osTimerDef(Timer2, timer_callback);

//====================================================
// nome
//====================================================
typedef enum{READY, RUNNING, WAITING} state;

typedef struct {
	 osThreadId *tid; 
	 int8_t dynamic_Priority;
   int8_t static_Priority;
	 state	task_state;// if it is READY, RUNNING or WAITING
	 uint16_t deadline; //in ticks
	 uint32_t initTime; //in ticks, time of execution
	 uint32_t executionTime;
	 uint32_t totalEstimateTime;
	 uint8_t execution_percentage; // Percentual de execução
	 bool isOver;
	 uint8_t frequency;
} taskDetails;

taskDetails taskA_details = {
	&tid_taskA,
	0,
	10,
	WAITING,
	2530,
	0,
	0,
	1488,
	0,
	false,
	8
};
taskDetails taskB_details = {
	&tid_taskB,
	0,
	0,
	WAITING,
	5838,
	0,
	0,
	3892,
	0,
	false,
  2
};
taskDetails taskC_details = {
	&tid_taskC,
	0,
	-30,
	WAITING,
	1096,
	0,
	0,
	843,
	0,
	false,
	5
};
taskDetails taskD_details = {
	&tid_taskD,
	0,
	0,
	WAITING,
	978,
	0,
	0,
	652,
	0,
	false,
	1
};
taskDetails taskE_details = {
	&tid_taskE,
	0,
	-30,
	WAITING,
	22708,
	0,
	0,
	17468,
	0,
	false,
  6
};
taskDetails taskF_details = {
	&tid_taskF,
	0,
	-100,
	WAITING,
	1592,
	0,
	0,
	1447,
	0,
	false,
	10
};
taskDetails taskDrawer_details = {
	&tid_drawer,
	0,
	127,
	WAITING,
	0,
	0,
	0,
	0,
	0,
	false,
	10
};



/*----------------------------------------------------------------------------
 *      Timer Function
 *---------------------------------------------------------------------------*/
void timer_callback(const void* args)
{

	static uint8_t timerA = 0, timerB = 0, timerC = 0, timerD = 0, timerE = 0, timerF = 0;
	const uint32_t 
		perA = 125/taskA_details.frequency, 
		perB = 125/taskB_details.frequency, 
		perC = 125/taskC_details.frequency, 
		perD = 125/taskD_details.frequency, 
		perE = 125/taskE_details.frequency, 
		perF = 125/taskF_details.frequency;
	timerA = (timerA + 1)% perA;
	timerB = (timerB + 1)% perB;
	timerC = (timerC + 1)% perC;
	timerD = (timerD + 1)% perD;
	timerE = (timerE + 1)% perE;
	timerF = (timerF + 1)% perF;
	
	if(timerA == 0 && taskA_details.task_state == WAITING){ taskA_details.task_state = READY; }
	if(timerB == 0 && taskB_details.task_state == WAITING){ taskB_details.task_state = READY; }
	if(timerC == 0 && taskC_details.task_state == WAITING){ taskC_details.task_state = READY; }
	if(timerD == 0 && taskD_details.task_state == WAITING){ taskD_details.task_state = READY; }
	if(timerE == 0 && taskE_details.task_state == WAITING){ taskE_details.task_state = READY; }
	if(timerF == 0 && taskF_details.task_state == WAITING){ taskF_details.task_state = READY; }
	osSignalSet(tid_scheduler, 0x0001);
	
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

void printLDC(tContext sContext ,tRectangle sRect){
	
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
	intToString(33, buf, 10, 10);
	Switch_On (LED_CLK);
	sRect.i16XMin = 15;
	sRect.i16YMin = 17;
	sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
	sRect.i16YMax = 128;
	GrContextForegroundSet(&sContext, ClrDarkBlue);
		GrRectFill(&sContext, &sRect);

	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrRectDraw(&sContext, &sRect);
	
	GrContextFontSet(&sContext, g_psFontCm12);
	intToString(33, buf, 10, 10);
//	GrStringDrawCentered(&sContext,(char*)buf, -1,
//											 GrContextDpyWidthGet(&sContext) / 2, 10, 0);
	GrFlush(&sContext);
	intToString(taskA_details.initTime, buf, 10, 10);
	GrContextFontSet(&sContext,g_psFontFixed6x8);
	GrStringDrawCentered(&sContext,"A", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 115)) + 10,0);
	GrStringDrawCentered(&sContext,"B", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 95)) + 10,0);
	GrStringDrawCentered(&sContext,"C", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);
	GrStringDrawCentered(&sContext,"D", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 55)) + 10,0);
	GrStringDrawCentered(&sContext,"E", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 35)) + 10,0);
	GrStringDrawCentered(&sContext,"F", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 17)) + 10,0);


	GrStringDrawCentered(&sContext,"PRI", -1,
											 GrContextDpyWidthGet(&sContext) - 100,
											 ((GrContextDpyHeightGet(&sContext)- 128)) + 10,0);
											 
	GrStringDrawCentered(&sContext,"%", -1,
											 GrContextDpyWidthGet(&sContext) - 80,
											 ((GrContextDpyHeightGet(&sContext)- 128)) + 10,0);
											 
	GrStringDrawCentered(&sContext,"STA", -1,
											 GrContextDpyWidthGet(&sContext) - 60,
											 ((GrContextDpyHeightGet(&sContext)- 128)) + 10,0);	
											 
	GrStringDrawCentered(&sContext,"DEAD", -1,
											 GrContextDpyWidthGet(&sContext) - 20,
											 ((GrContextDpyHeightGet(&sContext)- 128)) + 10,0);												 
											 


	GrFlush(&sContext);
	osDelay(5000);   
	Switch_Off(LED_CLK);

}


/*----------------------------------------------------------------------------
 *      Thread 1 'taskA': task A output
 *---------------------------------------------------------------------------*/
void taskA (void const *argument) {
	volatile int time;
  int32_t val;
	unsigned long a;
	int x;
	
	tContext sContext;
	tRectangle sRect;
	GrContextInit(&sContext, &g_sCfaf128x128x16);
  for (;;) {
   osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
//    Switch_On (LED_0);
                
		 taskA_details.initTime  = osKernelSysTick();
			for(x = 0 ; x<=256 ; x++)
			{
				a = (x+(x+2));
			}
	
		taskA_details.task_state = WAITING;
		printLDC( sContext ,sRect);
		osSignalSet(tid_scheduler, 0x0001);
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
//    Switch_On (LED_1);
		//osSignalSet(tid_taskC, 0x0001);
		taskB_details.initTime  = osKernelSysTick();
		for( x = 1 ; x<=16 ; x++)
			{
				b = (2^x)/fatorial(x);
			}
  
		taskB_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
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
		
		taskC_details.initTime = osKernelSysTick();
		for(x = 1 ; x<=72 ; x++)
			c = (x+1)/x;

		
		taskC_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
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
		taskD_details.initTime = osKernelSysTick();
		d = 1 + (5/fatorial(3))+(5/fatorial(5))+ (5/fatorial(7))+(5/fatorial(9));
		
		taskD_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
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
		taskE_details.initTime = osKernelSysTick();
		for(x = 1 ; x<=100 ;x++)
			e = x*PI2;
		
		taskE_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
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
		taskF_details.initTime  = osKernelSysTick();
		for(x = 1 ; x<=128 ; x++)
				f = (x*x*x)/(1<<x);
		
		taskF_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
  }
}

void drawer (void  const *argument) {

	tContext sContext;
	tRectangle sRect;
	GrContextInit(&sContext, &g_sCfaf128x128x16);
 
  while (1) {
	osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
	Switch_On (LED_CLK);
	sRect.i16XMin = 15;
	sRect.i16YMin = 20;
	sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
	sRect.i16YMax = 128;
	GrContextForegroundSet(&sContext, ClrDarkBlue);
		GrRectFill(&sContext, &sRect);

	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrRectDraw(&sContext, &sRect);
	
	GrContextFontSet(&sContext, g_psFontCm12);
	intToString(33, buf, 10, 10);
//	GrStringDrawCentered(&sContext,(char*)buf, -1,
//											 GrContextDpyWidthGet(&sContext) / 2, 10, 0);
	GrFlush(&sContext);
	intToString(taskA_details.initTime, buf, 10, 10);
	GrContextFontSet(&sContext,g_psFontFixed6x8);
	GrStringDrawCentered(&sContext,"A", -1,
											 GrContextDpyWidthGet(&sContext) - 118,
											 ((GrContextDpyHeightGet(&sContext)- 115)) + 10,
											 0);
	
	intToString(taskB_details.initTime, buf, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)buf, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 110)) + 10,
											 0);
	
	intToString(taskC_details.initTime, buf, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)buf, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 100)) + 10,
											 0);
	
	intToString(taskD_details.initTime, buf, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)buf, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 90)) + 10,0);
	
	
	intToString(taskE_details.initTime, buf, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)buf, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 80)) + 10,0);
	
	intToString(taskF_details.initTime, buf, 10, 10);
	GrContextFontSet(&sContext, g_psFontFixed6x8/*g_psFontFixed6x8*/);
	GrStringDrawCentered(&sContext,(char*)buf, -1,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext)- 70)) + 10,0);

	GrFlush(&sContext);
	osDelay(5000);   
	Switch_Off(LED_CLK);
	osSignalSet(tid_taskA, 0x0001);
  }
}

osThreadDef(taskA, osPriorityIdle, 1, 0);
osThreadDef(taskB, osPriorityIdle, 1, 0);
osThreadDef(taskC, osPriorityIdle, 1, 0);
osThreadDef(taskD, osPriorityIdle, 1, 0);
osThreadDef(taskE, osPriorityIdle, 1, 0);
osThreadDef(taskF, osPriorityIdle, 1, 0);
osThreadDef(drawer,  osPriorityIdle, 1, 0);

//----------------------------------

void swap(taskDetails** a, taskDetails** b){
	taskDetails* c = *a;
	*a = *b;
	*b = c;
}

void checkReady(taskDetails* tasksReady[7], uint8_t* sizeReady, taskDetails* tasksWaiting[6], uint8_t* sizeWaiting)
{
	int i, j;
	for(i=0; i<(*sizeWaiting); i++)
	{
		if(tasksWaiting[i]->task_state == READY)
		{
		  tasksReady[*sizeReady] = tasksWaiting[i];
			tasksWaiting[i] = NULL;
			for(j = i+1; j < 7 && tasksWaiting[j]; j++)
				swap(&(tasksWaiting[j-1]), &(tasksWaiting[j]));
			(*sizeReady)++;
			(*sizeWaiting)--;
			i--;
		}
	}
	
	for(i=0; i<(*sizeReady); i++)
	{
		if(tasksReady[i]->task_state == WAITING)
		{
		  tasksWaiting[*sizeWaiting] = tasksReady[i];
			tasksReady[i] = NULL;
			for(j = i+1; j < 7 && tasksReady[j]; j++)
				swap(&(tasksReady[j-1]), &(tasksReady[j]));
			(*sizeReady)--;
			(*sizeWaiting)++;
			i--;
		}
	}
}

int32_t getTotalPriority(taskDetails* details){
	return details->dynamic_Priority + details->static_Priority;
}

void policies(taskDetails* tasksReady[7], uint8_t* sizeReady){
	int i, j;
	for(i=0; i<(*sizeReady); i++)
	{
		if(tasksReady[i]->task_state == READY){
			if(tasksReady[i]->executionTime > tasksReady[i]->totalEstimateTime)
				  tasksReady[i]->dynamic_Priority = tasksReady[i]->dynamic_Priority -10;
			else
					tasksReady[i]->dynamic_Priority = 0;
		
		}
	}
}

taskDetails* nextRunning(taskDetails* tasksReady[7], uint8_t* sizeReady, taskDetails* lastRunning){
	uint8_t i;
	taskDetails* nextTask = lastRunning;
	if(*sizeReady > 0){
		nextTask = tasksReady[0];
		for(i = 1; i < *sizeReady; i++)
				if(getTotalPriority(tasksReady[i]) < getTotalPriority(nextTask))
					nextTask = tasksReady[i];
	}
	return nextTask;
}
void calcTime(taskDetails*  task){
		static uint32_t cur_time = 0;
		static uint32_t last_time = 0;
		cur_time =  cur_time;
		last_time = osKernelSysTick();
		task->executionTime += cur_time - last_time;
}


void scheduler()
{
	
	uint8_t i, sizeReady = 0, sizeWaiting = 7;
	taskDetails* runningTask = NULL;
	taskDetails* lastRunningTask = NULL;
	taskDetails* tasksReady[7] = {NULL};
	taskDetails* tasksWaiting[7] = 
	{
		&taskA_details, 
		&taskB_details, 
		&taskC_details, 
		&taskD_details, 
		&taskE_details, 
		&taskF_details, 
		&taskDrawer_details
	};
	const osThreadId* user_thread_ids[7] = 
	{
		&tid_taskA,
		&tid_taskB,
		&tid_taskC,
		&tid_taskD,
		&tid_taskE,
		&tid_taskF,
		&tid_drawer		
	};	
	
	while(1)
	{
	
		osSignalWait(0x0001, osWaitForever);
		checkReady(tasksReady, &sizeReady, tasksWaiting, &sizeWaiting);
		lastRunningTask = runningTask;
		if(lastRunningTask!= NULL )
		calcTime(lastRunningTask);
		policies(tasksReady, &sizeReady);
		runningTask = nextRunning(tasksReady, &sizeReady, lastRunningTask);
		
		if(!sizeReady)
			runningTask = NULL;
		for(i = 0; i < 7; i++)
			osThreadSetPriority(*(user_thread_ids[i]), osPriorityIdle);
		if(runningTask && runningTask->task_state != RUNNING)
			osSignalSet(*(runningTask->tid), 0x0001);
		if(lastRunningTask)
			lastRunningTask->task_state = READY;
		if(runningTask){
			runningTask->task_state = RUNNING;
			osThreadSetPriority(*(runningTask->tid), osPriorityNormal);
		}
		
		if (*(runningTask->tid) == tid_taskA) 	{Switch_On (LED_0); Switch_Off(LED_1); Switch_Off(LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskB) 	{Switch_Off(LED_0); Switch_On (LED_1); Switch_Off(LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskC) 	{Switch_On (LED_0); Switch_On (LED_1); Switch_Off(LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskD) 	{Switch_Off(LED_0); Switch_Off(LED_1); Switch_On (LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskE) 	{Switch_On (LED_0); Switch_Off(LED_1); Switch_On (LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskF) 	{Switch_Off(LED_0); Switch_On (LED_1); Switch_On (LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_drawer) 	{Switch_On (LED_0); Switch_On (LED_1); Switch_On (LED_2); Switch_Off(LED_3);}
		
	}
}
/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
	osKernelInitialize();
	
  SystemCoreClockUpdate();
  LED_Initialize();                         /* Initialize the LEDs           */
 	cfaf128x128x16Init();
  
  tid_taskA = osThreadCreate(osThread(taskA), NULL);
  tid_taskB = osThreadCreate(osThread(taskB), NULL);
  tid_taskC = osThreadCreate(osThread(taskC), NULL);
  tid_taskD = osThreadCreate(osThread(taskD), NULL);
	tid_taskE = osThreadCreate(osThread(taskE), NULL);
	tid_taskF = osThreadCreate(osThread(taskF), NULL);
	tid_drawer = osThreadCreate(osThread(drawer),  NULL);
	tid_scheduler = osThreadGetId();
	
	osThreadSetPriority(tid_scheduler, osPriorityHigh);
	osKernelStart();
	
	//Timer initialization
	timerScheduler = osTimerCreate (osTimer(Timer2), osTimerPeriodic, NULL);
	osTimerStart (timerScheduler, 8);    

	scheduler();
			
  osDelay(osWaitForever);
  while(1);
}