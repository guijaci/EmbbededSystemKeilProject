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
//Para uma frequência de sistema de 16MHz, uma divisão por 16000 equivale à um período de 1 milisegundo
#define SYSFREQ_FACTOR 16000
int i;

//Timer
void timer_callback(const void* args);						//Timer gatilgo das tarefas
void timer_execution_counter(const void* args);		//Timer para contagem do tempo de execução

osTimerId timerScheduler;                             
osTimerId timerExecutionCounter;                             
osTimerDef(Timer1, timer_callback);
osTimerDef(Timer2, timer_execution_counter);

//====================================================
// Estados das tarefas
//====================================================
typedef enum{READY, RUNNING, WAITING} state;

//Detalhes das Threads, inclui prioridades, frequencia, tempos de incio, deadline e execução
typedef struct {
	 osThreadId *tid; 
	 int32_t dynamic_Priority;
   int32_t static_Priority;
	 state	task_state;// if it is READY, RUNNING or WAITING
	 uint32_t deadline; //in ticks
	 uint32_t initTime; //in ticks, time of execution
	 uint32_t executionTime;
	 uint32_t totalEstimateTime;
	 uint8_t execution_percentage; // Percentual de execução
	 bool priorityChanged; //Flag indica se a prioridade dinamica ja foi modificada- tarefa atrasou
	 uint16_t frequency;
	 uint8_t deadline_percentage;
} taskDetails;

taskDetails taskA_details = {
	&tid_taskA,
	0,
	10,
	WAITING,
	80,//2530,
	0,
	0,
	47,//1488,
	0,
	false,
	8,
	70
};
taskDetails taskB_details = {
	&tid_taskB,
	0,
	0,
	WAITING,
	83,//5838,
	0,
	0,
	55,//3892,
	0,
	false,
  2,
	50
};
taskDetails taskC_details = {
	&tid_taskC,
	0,
	-30,
	WAITING,
	72,//1096,
	0,
	0,
	55,//843,
	0,
	false,
	5,
	30
};
taskDetails taskD_details = {
	&tid_taskD,
	0,
	0,
	WAITING,
	68, //978,
	0,
	0,
	45, //652,
	0,
	false,
	1,
	50
};
taskDetails taskE_details = {
	&tid_taskE,
	0,
	-30,
	WAITING,
	73,//22708,
	0,
	0,
	56,//17468,
	0,
	false,
  6,
	30
};
taskDetails taskF_details = {
	&tid_taskF,
	0,
	-100,
	WAITING,
	66,//1592,
	0,
	0,
	60,//1447,
	0,
	false,
	1,
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
	10,
	(uint8_t) -1
};

//Tarefa selecionada pelo escalonador
taskDetails* runningTask = NULL;
//Flag indica execução do escalonador
bool in_scheduler = false;
//Buffer para impressão de strings no LCD
char buf [10] ;

tContext sContext;
tRectangle sRect;

//====================================================
//Mail Queue
//====================================================
#define MAILQUEUE_OBJECTS      16                               
// number of Message Queue Objects
// object data type
typedef struct {                                                
	 taskDetails* task;
	 int32_t dynamic_Priority;
   int32_t static_Priority;
	 uint16_t deadline; //in ticks
	 uint32_t executionTime;
	 uint32_t totalEstimateTime;
	 uint8_t execution_percentage; // Percentual de execução
	 bool isError;
} MAILQUEUE_OBJ_t;
 
osMailQId qid_MailQueue;                                        
// mail queue id
 
osMailQDef (MailQueue, MAILQUEUE_OBJECTS, MAILQUEUE_OBJ_t);     
// mail queue object

//====================================================
//Flag que indica se o sistema esta rodando ou nao
//====================================================
bool systemRunning = true;

/*----------------------------------------------------------------------------
 *      Timer Function
 *---------------------------------------------------------------------------*/
void timer_callback(const void* args)
{
	static bool init = false;
	static uint8_t timerA = 0, timerB = 0, timerC = 0, timerD = 0, timerE = 0, timerF = 0;
	static uint32_t 
		perA,perB,perC,perD,perE,perF;
	
	//Inicia periodo de cada divisor de frequencia
	if(!init)
	{
		perA = 125/taskA_details.frequency; 
		perB = 125/taskB_details.frequency; 
		perC = 125/taskC_details.frequency; 
		perD = 125/taskD_details.frequency; 
		perE = 125/taskE_details.frequency; 
		perF = 125/taskF_details.frequency;
		init = true;
	}
	
	//Contadores dos divisores de frequência
	timerA = (timerA + 1)% perA;
	timerB = (timerB + 1)% perB;
	timerC = (timerC + 1)% perC;
	timerD = (timerD + 1)% perD;
	timerE = (timerE + 1)% perE;
	timerF = (timerF + 1)% perF;
	
	//Quando tempo estourar (== 0), muda a tarefa para READY e altera seu tempo inicio de gatilho
	if(timerA == 0 && taskA_details.task_state == WAITING)
	{
		taskA_details.initTime  = osKernelSysTick()/SYSFREQ_FACTOR;
		taskA_details.task_state = READY; 
	}
	if(timerB == 0 && taskB_details.task_state == WAITING)
	{  
		taskB_details.initTime  = osKernelSysTick()/SYSFREQ_FACTOR;
		taskB_details.task_state = READY; 
	}
	if(timerC == 0 && taskC_details.task_state == WAITING)
	{ 
		taskC_details.initTime  = osKernelSysTick()/SYSFREQ_FACTOR;
		taskC_details.task_state = READY; 
	}
	if(timerD == 0 && taskD_details.task_state == WAITING)
	{
		taskD_details.initTime  = osKernelSysTick()/SYSFREQ_FACTOR;
		taskD_details.task_state = READY;
	}
	if(timerE == 0 && taskE_details.task_state == WAITING)
	{ 
		taskE_details.initTime  = osKernelSysTick()/SYSFREQ_FACTOR;
		taskE_details.task_state = READY; 
	}
	if(timerF == 0 && taskF_details.task_state == WAITING)
	{
		taskF_details.initTime  = osKernelSysTick()/SYSFREQ_FACTOR;
		taskF_details.task_state = READY; 
	}
	
	//Troca contexto para escalonador (preempção)
	osSignalSet(tid_scheduler, 0x0001);
}

//Timer contador de tempo de execução
void timer_execution_counter(const void* args)
{
	//Incrementa tempo de execução se a tarefa esta corrente esta rodando e se o contexto não for do escalonador
	if(runningTask != NULL && runningTask->task_state == RUNNING && !in_scheduler)
		runningTask->executionTime++;
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

//Retorna string que representa o estado atual da thread
const char* verifyState(state  s){
	switch(s){
		case RUNNING:
			return "RN";
		case WAITING:
			return "WT";
		case READY:
			return "RD";
	}
}

/*----------------------------------------------------------------------------
 *      Thread 1 'taskA': task A output
 *---------------------------------------------------------------------------*/
void taskA (void const *argument) {
  MAILQUEUE_OBJ_t *pMail = 0;
	unsigned long a;
	int x;

	while (systemRunning == true) {
	 osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
	
		a = 0;
		for(x = 0 ; x<=256 ; x++)
			a += (x+(x+2));
		
    // Allocate memory
    pMail = osMailAlloc (qid_MailQueue, osWaitForever);         
 
		//Mail para drawer atualizar valores
    if (pMail) 
		{  // Set the mail content
      pMail->deadline = taskA_details.deadline;                                     
      pMail->dynamic_Priority = taskA_details.dynamic_Priority;
			pMail->executionTime = taskA_details.executionTime;
			pMail->execution_percentage = taskA_details.execution_percentage;
			pMail->static_Priority = taskA_details.static_Priority;
			pMail->task = &taskA_details;
			pMail->totalEstimateTime = taskA_details.totalEstimateTime; 
			pMail->isError = NULL;
			taskDrawer_details.task_state = READY;
			
      osMailPut (qid_MailQueue, pMail);                         
      // Send Mail
    }		
		
		//Controle retorna para escalonador
		taskA_details.task_state = WAITING;
    osSignalSet(tid_scheduler, 0x0001);
  }
	
	osDelay(osWaitForever);
	
}

/*----------------------------------------------------------------------------
 *      Thread 2 'taskB': task B output
 *---------------------------------------------------------------------------*/
void taskB (void const *argument) {
	MAILQUEUE_OBJ_t *pMail = 0;
	unsigned long b;
	int x;

	while (systemRunning == true) {
		osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */

		b = 0;
		for( x = 1 ; x<=16 ; x++)
			b += (2^x)/fatorial(x);
		
    // Allocate memory
    pMail = osMailAlloc (qid_MailQueue, osWaitForever);         
 
		//Mail para drawer atualizar valores
    if (pMail) 
		{  // Set the mail content
			pMail->deadline = taskB_details.deadline;                                     
      pMail->dynamic_Priority = taskB_details.dynamic_Priority;
			pMail->executionTime = taskB_details.executionTime;
			pMail->execution_percentage = taskB_details.execution_percentage;
			pMail->static_Priority = taskB_details.static_Priority;
			pMail->task = &taskB_details;
			pMail->totalEstimateTime = taskB_details.totalEstimateTime;
      pMail->isError = NULL;
			
			taskDrawer_details.task_state = READY;
			
			osMailPut (qid_MailQueue, pMail);                         
      // Send Mail
    }
		
		//Controle retorna para escalonador
		taskB_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
  }
	osDelay(osWaitForever);
}

/*----------------------------------------------------------------------------
 *      Thread 3 'taskC': task C output
 *---------------------------------------------------------------------------*/
void taskC (void const *argument) {
  MAILQUEUE_OBJ_t *pMail = 0;
	unsigned long c;
	int x;
 
	while (systemRunning == true) {
		osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */

		c = 0;
		for(x = 1 ; x<=72 ; x++)
			c += (x+1)/x;

    // Allocate memory
    pMail = osMailAlloc (qid_MailQueue, osWaitForever);         
 
		//Mail para drawer atualizar valores
    if (pMail) 
		{  // Set the mail content
      pMail->deadline = taskC_details.deadline;                                     
      pMail->dynamic_Priority = taskC_details.dynamic_Priority;
			pMail->executionTime = taskC_details.executionTime;
			pMail->execution_percentage = taskC_details.execution_percentage;
			pMail->static_Priority = taskC_details.static_Priority;
			pMail->task = &taskC_details;
			pMail->totalEstimateTime = taskC_details.totalEstimateTime; 
      pMail->isError = NULL;
			
			taskDrawer_details.task_state = READY;
		
			osMailPut (qid_MailQueue, pMail);                         
      // Send Mail
    }
 
		//Controle retorna para escalonador
		taskC_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
		
  }
	osDelay(osWaitForever);
}

/*----------------------------------------------------------------------------
 *      Thread 4 'taskD': task D output
 *---------------------------------------------------------------------------*/
void taskD (void  const *argument) {
  MAILQUEUE_OBJ_t *pMail = 0;
	unsigned long  d;
	
	while (systemRunning == true) {
		osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
		
		d = 1 + (5/fatorial(3))+(5/fatorial(5))+ (5/fatorial(7))+(5/fatorial(9));

    // Allocate memory
		pMail = osMailAlloc (qid_MailQueue, osWaitForever);         
 
		//Mail para drawer atualizar valores
    if (pMail) {  // Set the mail content
      pMail->deadline = taskD_details.deadline;                                     
      pMail->dynamic_Priority = taskD_details.dynamic_Priority;
			pMail->executionTime = taskD_details.executionTime;
			pMail->execution_percentage = taskD_details.execution_percentage;
			pMail->static_Priority = taskD_details.static_Priority;
			pMail->task = &taskD_details;
			pMail->totalEstimateTime = taskD_details.totalEstimateTime; 
			pMail->isError = NULL;

			taskDrawer_details.task_state = READY;
		
      osMailPut (qid_MailQueue, pMail);                         
      // Send Mail
    }
 
		//Controle retorna para escalonador
		taskD_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
  }
	osDelay(osWaitForever);
}

void taskE (void  const *argument) {
	MAILQUEUE_OBJ_t *pMail = 0;
	int x;
	unsigned long e;
 
	while (systemRunning == true) {
		osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
	
		e = 0;
		for(x = 1 ; x<=100 ;x++)
			e += x*PI2;
		
    // Allocate memory
		pMail = osMailAlloc (qid_MailQueue, osWaitForever);         
 
		//Mail para drawer atualizar valores
    if (pMail) 
		{  // Set the mail content
      pMail->deadline = taskE_details.deadline;                                     
      pMail->dynamic_Priority = taskE_details.dynamic_Priority;
			pMail->executionTime = taskE_details.executionTime;
			pMail->execution_percentage = taskE_details.execution_percentage;
			pMail->static_Priority = taskE_details.static_Priority;
			pMail->task = &taskE_details;
			pMail->totalEstimateTime = taskE_details.totalEstimateTime; 
      pMail->isError = NULL;
			
			taskDrawer_details.task_state = READY;
			
			osMailPut (qid_MailQueue, pMail);                         
      // Send Mail
    }
 
		//Controle retorna para escalonador
    taskE_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
  }
	 osDelay(osWaitForever);
}



void taskF(void  const *argument) {
  MAILQUEUE_OBJ_t *pMail = 0;
	int x;
	unsigned long f;
   
	while (systemRunning == true) {
		osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */

		f = 0;
		for(x = 1 ; x<=128 ; x++)
				f += (x*x*x)/(1<<x);
			
    // Allocate memory
    pMail = osMailAlloc (qid_MailQueue, osWaitForever);         
 
		//Mail para drawer atualizar valores
    if (pMail) 
		{  // Set the mail content
      pMail->deadline = taskF_details.deadline;                                     
      pMail->dynamic_Priority = taskF_details.dynamic_Priority;
			pMail->executionTime = taskF_details.executionTime;
			pMail->execution_percentage = taskF_details.execution_percentage;
			pMail->static_Priority = taskF_details.static_Priority;
			pMail->task = &taskF_details;
			pMail->totalEstimateTime = taskF_details.totalEstimateTime; 
			pMail->isError = NULL;
			
			taskDrawer_details.task_state = READY;		

			osMailPut (qid_MailQueue, pMail);                         
      // Send Mail
    }
 
		//Controle retorna para escalonador
    taskF_details.task_state = WAITING;
		osSignalSet(tid_scheduler, 0x0001);
  }
	osDelay(osWaitForever);
  
}

void drawer (void  const *argument) {
	taskDetails *task_details;
	MAILQUEUE_OBJ_t  *pMail = 0;
  osEvent           evt;

  while (systemRunning == true) {
		evt = osMailGet (qid_MailQueue, osWaitForever);             
		// wait for mail
		if (evt.status == osEventMail) {
			pMail = evt.value.p;
		if (pMail) {
			//Impressão das tasks:
			task_details = pMail->task;
			
			//ERRO - Task realtime passou do limite
			if(pMail->isError == true)
			{
				sRect.i16XMin = 15;
				sRect.i16YMin = 17;
				sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
				sRect.i16YMax = 128;
				
				GrContextForegroundSet(&sContext, ClrBlack);
				GrContextBackgroundSet(&sContext, ClrBlack);
				GrRectFill(&sContext, &sRect);
				GrContextForegroundSet(&sContext, ClrWhite);
				
				GrStringDrawCentered(&sContext,"Erro!", -1,
														 GrContextDpyWidthGet(&sContext) - 60,
														 ((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);
				
				//Flag de sistema resetada - sistema termina
				systemRunning = false;
			}
			else 
			{
				//Dados Task A
				if(task_details->tid == &tid_taskA)
				{
					sRect.i16XMin = 15;
					sRect.i16YMin = 17;
					sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
					sRect.i16YMax = 35;
					GrContextForegroundSet(&sContext, ClrBlack);
					GrContextBackgroundSet(&sContext, ClrBlack);
					GrRectFill(&sContext, &sRect);
					GrContextForegroundSet(&sContext, ClrWhite);
					
					intToString(pMail->static_Priority + pMail->dynamic_Priority, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 100, 
					((GrContextDpyHeightGet(&sContext)- 115)) + 10,0);	
					
					intToString(pMail->executionTime*100/pMail->totalEstimateTime, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 80, 
					((GrContextDpyHeightGet(&sContext)- 115)) + 10,0);	
					
					
					GrStringDrawCentered(&sContext,verifyState(task_details->task_state), -1,
														 GrContextDpyWidthGet(&sContext) - 60, 
					((GrContextDpyHeightGet(&sContext)- 115)) + 10,0);	
					
					intToString(pMail->deadline, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 20, 
					((GrContextDpyHeightGet(&sContext)- 115)) + 10,0);	
							
				}
				//Dados Task B
				if(task_details->tid == &tid_taskB)
				{
					sRect.i16XMin = 15;
					sRect.i16YMin = 35;
					sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
					sRect.i16YMax = 53;
					GrContextForegroundSet(&sContext, ClrBlack);
					GrContextBackgroundSet(&sContext, ClrBlack);
					GrRectFill(&sContext, &sRect);
					GrContextForegroundSet(&sContext, ClrWhite);
				
					intToString(pMail->static_Priority + pMail->dynamic_Priority, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 100, 
					((GrContextDpyHeightGet(&sContext)- 95)) + 10,0);	
					
						intToString(pMail->executionTime*100/pMail->totalEstimateTime, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 80, 
					((GrContextDpyHeightGet(&sContext)- 95)) + 10,0);	
					
					
					GrStringDrawCentered(&sContext,verifyState(task_details->task_state), -1,
														 GrContextDpyWidthGet(&sContext) - 60, 
					((GrContextDpyHeightGet(&sContext)- 95)) + 10,0);	
					
					intToString(pMail->deadline, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) -20, 
					((GrContextDpyHeightGet(&sContext) -95)) + 10,0);	
				}
				//Dados Task C
				if(task_details->tid == &tid_taskC){
					sRect.i16XMin = 15;
					sRect.i16YMin = 53;
					sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
					sRect.i16YMax = 70;
					GrContextForegroundSet(&sContext, ClrBlack);
					GrContextBackgroundSet(&sContext, ClrBlack);
					GrRectFill(&sContext, &sRect);

					
					GrContextForegroundSet(&sContext, ClrWhite);
					intToString(pMail->static_Priority+pMail->dynamic_Priority, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 100, 
					((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);	
					
						intToString(pMail->executionTime*100/pMail->totalEstimateTime, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 80, 
					((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);	
					
					GrStringDrawCentered(&sContext,verifyState(task_details->task_state), -1,
														 GrContextDpyWidthGet(&sContext) - 60, 
					((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);	
					
					intToString(pMail->deadline, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 20, 
					((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);	
				}
				//Dados Task D
				if (task_details->tid == &tid_taskD){
					sRect.i16XMin = 15;
					sRect.i16YMin = 70;
					sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
					sRect.i16YMax = 91;
					GrContextForegroundSet(&sContext, ClrBlack);
					GrContextBackgroundSet(&sContext, ClrBlack);
					GrRectFill(&sContext, &sRect);

					GrContextForegroundSet(&sContext, ClrWhite);
					intToString(pMail->static_Priority + pMail->dynamic_Priority, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 100, 
					((GrContextDpyHeightGet(&sContext)- 55)) + 10,0);	
					
						intToString(pMail->executionTime*100/pMail->totalEstimateTime, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 80, 
					((GrContextDpyHeightGet(&sContext)- 55)) + 10,0);	
				
					
					GrStringDrawCentered(&sContext,verifyState(task_details->task_state), -1,
														 GrContextDpyWidthGet(&sContext) - 60, 
					((GrContextDpyHeightGet(&sContext)- 55)) + 10,0);	
					
					intToString(pMail->deadline, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 20, 
					((GrContextDpyHeightGet(&sContext)- 55)) + 10,0);	
				}
				//Dados Task E
				if (task_details->tid == &tid_taskE){
					sRect.i16XMin = 15;
					sRect.i16YMin = 91;
					sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
					sRect.i16YMax = 110;
					GrContextForegroundSet(&sContext, ClrBlack);
					GrContextBackgroundSet(&sContext, ClrBlack);
					GrRectFill(&sContext, &sRect);
					GrContextForegroundSet(&sContext, ClrWhite);
					
					intToString(pMail->static_Priority + pMail->dynamic_Priority, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 100, 
					((GrContextDpyHeightGet(&sContext)- 35)) + 10,0);	
					
						intToString(pMail->executionTime*100/pMail->totalEstimateTime, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 80, 
					((GrContextDpyHeightGet(&sContext)- 35)) + 10,0);	
				
			
					GrStringDrawCentered(&sContext,verifyState(task_details->task_state), -1,
														 GrContextDpyWidthGet(&sContext) - 60, 
					((GrContextDpyHeightGet(&sContext)- 35)) + 10,0);	
					
					intToString(pMail->deadline, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 20, 
					((GrContextDpyHeightGet(&sContext)- 35)) + 10,0);	
				}
				//Dados Task F
				if(task_details->tid == &tid_taskF){
					//Desenho
					sRect.i16XMin = 15;
					sRect.i16YMin = 110;
					sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
					sRect.i16YMax = 128;
					GrContextForegroundSet(&sContext, ClrBlack);
					GrContextBackgroundSet(&sContext, ClrBlack);
					GrRectFill(&sContext, &sRect);
					GrContextForegroundSet(&sContext, ClrWhite);
					
					intToString(pMail->static_Priority + pMail->dynamic_Priority, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 100, 
					((GrContextDpyHeightGet(&sContext)- 17)) + 10,0);	
					
						intToString(pMail->executionTime*100/pMail->totalEstimateTime, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 80, 
					((GrContextDpyHeightGet(&sContext)- 17)) + 10,0);	
					
				
					GrStringDrawCentered(&sContext,verifyState(task_details->task_state), -1,
														 GrContextDpyWidthGet(&sContext) - 60, 
					((GrContextDpyHeightGet(&sContext)- 17)) + 10,0);	
					
					intToString(pMail->deadline, buf, 10, 10);
					GrStringDrawCentered(&sContext,(char*)buf, -1,
														 GrContextDpyWidthGet(&sContext) - 20, 
					((GrContextDpyHeightGet(&sContext)- 17)) + 10,0);	
					
					}
				}
			
				//Controle retorna para escalonador
				osSignalSet(tid_scheduler, 0x0001);
        osMailFree (qid_MailQueue, pMail);                      
      }
    }
	}
	osDelay(osWaitForever);
}

osThreadDef(taskA, osPriorityIdle, 1, 0);
osThreadDef(taskB, osPriorityIdle, 1, 0);
osThreadDef(taskC, osPriorityIdle, 1, 0);
osThreadDef(taskD, osPriorityIdle, 1, 0);
osThreadDef(taskE, osPriorityIdle, 1, 0);
osThreadDef(taskF, osPriorityIdle, 1, 0);
osThreadDef(drawer,  osPriorityIdle, 1, 0);

//----------------------------------
//Funções auxiliares de Escalonador

//Troca 2 variaveis taskDetails
void swap(taskDetails** a, taskDetails** b){
	taskDetails* c = *a;
	*a = *b;
	*b = c;
}

//Verifica threads na fila de prontas e na fila de espera para reorganiza-las, colocando-as nas filas corretas
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

//Retorna prioridade total de uma thread
int32_t getTotalPriority(taskDetails* details){
	return details->dynamic_Priority + details->static_Priority;
}

//Aplica politicas correspondentes para aumentar ou diminuir prioridades dinâmicas
bool policies(taskDetails* tasksReady[7], uint8_t* sizeReady, taskDetails* runningTask, taskDetails* lastRunningTask)
{
	int i, j;
	//Variavel representa quanto tempo desde mudar estado para READY de uma thread passou
  uint32_t lifespam;
	
	//Para todas tarefas na fila de prontas que nao estão em execução...
	for(i = 0 ; i <(*sizeReady) ; i++)
	{
		if(tasksReady[i]->task_state != RUNNING)	
		{	
			//... caso o tempo estimado seja 0 (caso do drawer), ignore ...
			if(tasksReady[i]->totalEstimateTime == 0 )
				continue;
			lifespam = osKernelSysTick()/SYSFREQ_FACTOR > tasksReady[i]->initTime ? osKernelSysTick()/SYSFREQ_FACTOR - tasksReady[i]->initTime : (UINT32_MAX + osKernelSysTick())/SYSFREQ_FACTOR - tasksReady[i]->initTime;
			//... caso tempo de vida tenha ultrapassado deadline ...
			if(lifespam > tasksReady[i]->deadline && tasksReady[i]->deadline_percentage != (uint8_t)-1)
			{
				//... e for realtime, retorne falso para sinalizar erro de sistema ...
				if(tasksReady[i]->static_Priority == -100 )
				{
					//return false; //Comentado para visualizar sistema em execução
				}
				else
				{
					//... se nao realtime, aumente sua prioridade 
					//(apenas se já não haver sido aumentado durante execução desta tarefa)
					if(!tasksReady[i]->priorityChanged)
					{
						tasksReady[i]->dynamic_Priority -= 10; //segundo paragrafo	
						tasksReady[i]->priorityChanged = true;
					}
				}
			}
		}
	}
	
	lifespam = osKernelSysTick()/SYSFREQ_FACTOR > runningTask->initTime ? osKernelSysTick()/SYSFREQ_FACTOR - runningTask->initTime : (UINT32_MAX + osKernelSysTick())/SYSFREQ_FACTOR - runningTask->initTime;
	//Se a tarefa corrente terminar (se runningTask estiver em WAITING, ela terminou) ...
	if(runningTask != NULL && runningTask->task_state == WAITING)
	{
		//... permita que a prioridade de todas as tarefas da fila de prontas tenham suas prioridades alteradas novamente ...
		for(i = 0 ; i <(*sizeReady) ; i++)
			tasksReady[i]->priorityChanged = false;
		
		//... reinicie sua prioridade dinâmica (evita starvation)...
		runningTask->dynamic_Priority = 0;
		
		//... se existe estimativa de tempo de execução (caso não seja tarefa drawer) ...
		if(runningTask->totalEstimateTime > 0)
		{
			//... se esta tarefa terminou com depois do seu deadline ...
			if(lifespam > runningTask->deadline && tasksReady[i]->deadline_percentage != (uint8_t)-1)
			{
				//... e for realtime, retorne falso para sinalizar erro de sistema ...
				if(runningTask->static_Priority == -100 )
				{
					//return false; //Comentado para visualizar sistema em execução
				}
				else
				{
					//... se nao realtime, aumente sua prioridade 
					runningTask->dynamic_Priority -= 10; //segundo paragrafo
				}
			}
			
			//Politica para tarefa que terminam antes da metade de seu deadline, para diminuir sua prioridade
			if(lifespam < (runningTask->deadline + runningTask->totalEstimateTime)/2 && tasksReady[i]->deadline_percentage != (uint8_t)-1)
			{
				if(runningTask->static_Priority != -100 && runningTask->deadline != 0)
				{
					runningTask->dynamic_Priority += 10;
				}
			}
		}
	}
	
	//Finalização OK, não houve erros
	return true;
}

//Procura na fila de prontas a tarefa de maior prioridade para ser a próxima à executar
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

//Escalonador, gerencia tarefas prontas para executar e em espera, 
//"agendando" execução de tarefas com base em sua politica 
void scheduler()
{
	bool policiesResult;
	//mail queue
	MAILQUEUE_OBJ_t  *pMail = 0;
	uint8_t i, sizeReady = 0, sizeWaiting = 7;
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
	
	while(systemRunning == true){
		osSignalWait(0x0001, osWaitForever);
		//Executa apenas após preempção ou final de execução de tarefa
		in_scheduler = true;
		//Organiza fila de prontas e em espera com base nos estados das threads
		checkReady(tasksReady, &sizeReady, tasksWaiting, &sizeWaiting);
		
		lastRunningTask = runningTask;	
		//Aplica politicas para modificar prioridades
		policiesResult = policies(tasksReady, &sizeReady, runningTask, lastRunningTask);
		
		//Se retorna falso, houve um erro, e deverá ser enviado mensagem ao drawer
		if(!policiesResult)
		{
			pMail = osMailAlloc (qid_MailQueue, osWaitForever); 	
			if (pMail)				
			{  // Set the mail content
					pMail->isError = true;
					osMailPut (qid_MailQueue, pMail);                         
					// Send Mail
			
			}
		}
		
		//Seleciona proxima tarefa para executar
		runningTask = nextRunning(tasksReady, &sizeReady, lastRunningTask);
		
		//Depois de executar uma tarefa, reinicia tempos
		if(lastRunningTask->task_state == WAITING )
			lastRunningTask->executionTime = 0;				
		
		//Garante que se não houver nenhuma tarefa pronta nao será executado nenhuma tarefa
		if(!sizeReady)
			runningTask = NULL;
		//Diminui prioridades das outras tarefas para o minimo possivel
		for(i = 0; i < 7; i++)
			osThreadSetPriority(*(user_thread_ids[i]), osPriorityIdle);
		//Ultima tarefa fica em READY
		if(lastRunningTask)
			lastRunningTask->task_state = READY;
		//Nova tarefa fica em RUNNING, tem uma prioridade maior que as outras e é sinalizada
		if(runningTask){
			runningTask->task_state = RUNNING;
			osThreadSetPriority(*(runningTask->tid), osPriorityNormal);
			osSignalSet(*(runningTask->tid), 0x0001);
		}
		
		//Acende LEDs indicativos de tarefas selecionadas
		if (*(runningTask->tid) == tid_taskA) 	{Switch_On (LED_0); Switch_Off(LED_1); Switch_Off(LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskB) 	{Switch_Off(LED_0); Switch_On (LED_1); Switch_Off(LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskC) 	{Switch_On (LED_0); Switch_On (LED_1); Switch_Off(LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskD) 	{Switch_Off(LED_0); Switch_Off(LED_1); Switch_On (LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskE) 	{Switch_On (LED_0); Switch_Off(LED_1); Switch_On (LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_taskF) 	{Switch_Off(LED_0); Switch_On (LED_1); Switch_On (LED_2); Switch_Off(LED_3);}
		if (*(runningTask->tid) == tid_drawer) 	{Switch_On (LED_0); Switch_On (LED_1); Switch_On (LED_2); Switch_Off(LED_3);}
		
		in_scheduler = false;
	}
}


//Inicializa contexto para display e desenha tabela
void init_display(){
	
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext,g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
	
	//Escreve menu lateral:
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
	
	//Escreve menu superior:
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
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
	osKernelInitialize();
	
  SystemCoreClockUpdate();
  LED_Initialize();                         /* Initialize the LEDs           */
 	cfaf128x128x16Init();
	
	init_display();
	
  tid_taskA = osThreadCreate(osThread(taskA), NULL);
  tid_taskB = osThreadCreate(osThread(taskB), NULL);
  tid_taskC = osThreadCreate(osThread(taskC), NULL);
  tid_taskD = osThreadCreate(osThread(taskD), NULL);
	tid_taskE = osThreadCreate(osThread(taskE), NULL);
	tid_taskF = osThreadCreate(osThread(taskF), NULL);
	tid_drawer = osThreadCreate(osThread(drawer), NULL);
	tid_scheduler = osThreadGetId();
	
	//Mail queue initialization
	qid_MailQueue = osMailCreate (osMailQ(MailQueue), NULL);      
  // create mail queue
	osThreadSetPriority(tid_scheduler, osPriorityHigh);
	
	//Timer initialization
	timerScheduler 				= osTimerCreate (osTimer(Timer1), osTimerPeriodic, NULL);
	timerExecutionCounter = osTimerCreate (osTimer(Timer2), osTimerPeriodic, NULL);
	osTimerStart (timerScheduler, 				8);    
	osTimerStart (timerExecutionCounter,  1);    
	
	osKernelStart();

	scheduler();
	osThreadTerminate(tid_taskA);
	osThreadTerminate(tid_taskB);
	osThreadTerminate(tid_taskC);
	osThreadTerminate(tid_taskD);
	osThreadTerminate(tid_taskE);
	osThreadTerminate(tid_taskF);
	osThreadTerminate(tid_drawer);
	
  osDelay(osWaitForever);
  while(1);
}