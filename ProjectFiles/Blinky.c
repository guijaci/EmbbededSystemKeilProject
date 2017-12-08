/*-------------4---------------------------------------------------------------
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
/*----------------------------------------------------------------------------
 * include libraries from drivers
 *----------------------------------------------------------------------------*/
#include "rgb.h"
#include "cfaf128x128x16.h"
#include "servo.h"
#include "temp.h"
#include "opt.h"
#include "buttons.h"
#include "buzzer.h"

osThreadId tid_buzzer;                 /* Thread id of thread: buzzer     			 	*/
osThreadId tid_motor;                  /* Thread id of thread: motor       				*/
osThreadId tid_rgb;                    /* Thread id of thread: rgb 	       				*/
osThreadId tid_accel;                  /* Thread id of thread: accelerometer      */
osThreadId tid_temp;                   /* Thread id of thread: temperture	        */
osThreadId tid_light;                  /* Thread id of thread: microphone		      */
osThreadId tid_mic;   


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
  osSignalSet(tid, 0x0001);                 /* set signal to thread 'thread' */
  osDelay(500);                             /* delay 500ms                   */
}

/*----------------------------------------------------------------------------
 *      Thread 1 't_buzzer': Phase A output
 *---------------------------------------------------------------------------*/

void t_buzzer(void const *argument){
	

	
}
	
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/

void t_rgb(void const *argument){}
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
	
void t_mic(void const *argument){}
	
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
	
void t_light(void const *argument){}
	
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
	
void t_temp(void const *argument){}
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/

void t_motor(void const *argument){}
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
	
void t_accel(void const *argument){}


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


/*----------------------------------------------------------------------------
 *      Thread 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
void clock (void  const *argument) {
	tContext sContext;
	tRectangle sRect;
	int16_t temp,temp2,joystic_x, joystic_y, acc_x, acc_y,acc_z,mic;
	long double teste;
	char buf[10];
	bool trig = true;
	int cont=0;
	bool button1;
	bool button2;
	
	
	GrContextInit(&sContext, &g_sCfaf128x128x16);

	sRect.i16XMin = 0;
	sRect.i16YMin = 0;
	sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
	sRect.i16YMax = 115;
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	temp = temp_read();
	

  for (;;) {
    osSignalWait(0x0100, osWaitForever);    /* wait for an event flag 0x0100 */
		temp = opt_read();
		button1 = button_read_s1();
		button2 = button_read_s2();
		joystic_x = joy_read_x();
		joystic_y = joy_read_y();
		acc_x = accel_read_x();
		acc_y = accel_read_y();
		acc_z = accel_read_z();
		mic = mic_read();
		buzzer_read();
		intToString((uint16_t)temp, buf, 10, 10);
		GrContextForegroundSet(&sContext, ClrDarkBlue);
		GrRectFill(&sContext, &sRect);
		GrContextForegroundSet(&sContext, ClrWhite);

		temp2 = opt_get_lux();
		intToString((uint16_t)temp2, buf, 10, 10);
		GrStringDrawCentered(&sContext, buf, -1,
												 GrContextDpyWidthGet(&sContext) / 2, 25, 0);
		if(button1)
		GrStringDrawCentered(&sContext,(char*)"B1 Pushed", -1,
												 GrContextDpyWidthGet(&sContext) / 2, 35, 0);
		if(button2)
		GrStringDrawCentered(&sContext,(char*)"B2 Pushed", -1,
												 GrContextDpyWidthGet(&sContext) / 2, 45, 0);
		
		intToString((uint16_t)joystic_x, buf, 10, 10);
		GrStringDrawCentered(&sContext,buf, -1,
												 GrContextDpyWidthGet(&sContext) / 2, 55, 0);
	  intToString((uint16_t)joystic_y, buf, 10, 10);
		GrStringDrawCentered(&sContext,buf, -1,
												 GrContextDpyWidthGet(&sContext) / 2, 65, 0);
		
		
		intToString((uint16_t)acc_x, buf, 10, 10);
		GrStringDrawCentered(&sContext,buf, -1,
												 GrContextDpyWidthGet(&sContext) / 2, 75, 0);
		
		intToString((uint16_t)acc_y, buf, 10, 10);
		GrStringDrawCentered(&sContext,buf, -1,
												 GrContextDpyWidthGet(&sContext) / 2, 85, 0);
		
		intToString((uint16_t)acc_z, buf, 10, 10);
		GrStringDrawCentered(&sContext,buf, -1,
												 GrContextDpyWidthGet(&sContext) / 2, 95, 0);
		
		intToString((uint16_t)mic, buf, 10, 10);
		GrStringDrawCentered(&sContext,buf, -1,
												 GrContextDpyWidthGet(&sContext) / 2, 105, 0);
		
		GrFlush(&sContext);
    osDelay(5000);  
    Switch_Off(LED_CLK);
  }
}

osThreadDef(t_buzzer, 		osPriorityNormal, 1, 0);
osThreadDef(t_rgb		, 		osPriorityNormal, 1, 0);
osThreadDef(t_mic		, 		osPriorityNormal, 1, 0);
osThreadDef(t_light	,			osPriorityNormal, 1, 0);
osThreadDef(t_temp	, 	 	osPriorityNormal, 1, 0);
osThreadDef(t_motor	,  		osPriorityNormal, 1, 0);
osThreadDef(t_accel	,  		osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
	int16_t angle = 0, inc = 1;
	osKernelInitialize();
	cfaf128x128x16Init();
	rgb_init();
	servo_init();
	temp_init();
	opt_init();
	button_init();
	joy_init();
	accel_init();
	mic_init();
	buzzer_init();
	
  tid_buzzer = osThreadCreate(osThread(t_buzzer), NULL);
  tid_rgb 	 = osThreadCreate(osThread(t_rgb), 		NULL);
  tid_mic 	 = osThreadCreate(osThread(t_mic), 		NULL);
  tid_light  = osThreadCreate(osThread(t_light), 	NULL);
  tid_temp   = osThreadCreate(osThread(t_temp),  	NULL);
	tid_motor  = osThreadCreate(osThread(t_motor),  NULL);
	tid_accel  = osThreadCreate(osThread(t_accel),  NULL);
	
	osKernelStart();
//	
//	osSignalSet(tid_phaseA, 0x0001);          /* set signal to phaseA thread   */

	while(true){
		servo_write_degree180(angle);
		angle += inc;
		if(angle == 90 || angle == -90) 
			inc = -inc;	
		osDelay(200);
	}
  osDelay(osWaitForever);
  while(1);
}
