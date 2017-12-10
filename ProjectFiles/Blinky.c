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
#include "string.h"
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
#include "joy.h"
#include "mic.h"
#include "accel.h"
#include "led.h"

osThreadId tid_buzzer;                 /* Thread id of thread: buzzer     			 	*/
osThreadId tid_servo;                  /* Thread id of thread: motor       				*/
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

//To print on the screen
tContext sContext;
tRectangle sRect;

char pbuf[10];
/*----------------------------------------------------------------------------
 *  Transforming int to string
 *---------------------------------------------------------------------------*/
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
 *    Initializations
 *---------------------------------------------------------------------------*/

void init_all(){
	cfaf128x128x16Init();
	joy_init();
	accel_init();
	buzzer_init(); 
	button_init();
	mic_init();
	rgb_init();
	servo_init();
	temp_init();
	opt_init();
	led_init();
}
/*----------------------------------------------------------------------------
 *    Sidelong menu with thread's name
 *---------------------------------------------------------------------------*/

void init_sidelong_menu(){
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext,g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
	
	//Escreve menu lateral:
	GrStringDrawCentered(&sContext,"B", -1,
								 GrContextDpyWidthGet(&sContext) - 120,
								 ((GrContextDpyHeightGet(&sContext)- 128)) + 10,0);
	GrStringDrawCentered(&sContext,"S", -1,
								 GrContextDpyWidthGet(&sContext) - 120,
								 ((GrContextDpyHeightGet(&sContext)- 115)) + 10,0);
	GrStringDrawCentered(&sContext," RGB", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 95)) + 10,0);
	GrStringDrawCentered(&sContext,"A", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);
	GrStringDrawCentered(&sContext,"T", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 55)) + 10,0);
	GrStringDrawCentered(&sContext,"L", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 35)) + 10,0);
	GrStringDrawCentered(&sContext,"M", -1,
											 GrContextDpyWidthGet(&sContext) - 120,
											 ((GrContextDpyHeightGet(&sContext)- 17)) + 10,0);

}
/*----------------------------------------------------------------------------
 *      Switch LED on
 *---------------------------------------------------------------------------*/
void Switch_On (unsigned char led) {
  if (led != LED_CLK) led_on (led);
}

/*----------------------------------------------------------------------------
 *      Switch LED off
 *---------------------------------------------------------------------------*/
void Switch_Off (unsigned char led) {
  if (led != LED_CLK) led_off (led);
}


/*----------------------------------------------------------------------------
 *      Function 'signal_func' called from multiple threads
 *---------------------------------------------------------------------------*/
void signal_func (osThreadId tid)  {
  osSignalSet(tid, 0x0001);                 /* set signal to thread 'thread' */
}

/*----------------------------------------------------------------------------
 *      Function 'set_thread_status'
*			status: 1 for Running, and 0 for Waiting
 *---------------------------------------------------------------------------*/
void thread_status(osThreadId tid, int8_t status)  {
	 
}
/*----------------------------------------------------------------------------
 *      Thread 1 't_buzzer': Phase A output
 *---------------------------------------------------------------------------*/

void t_buzzer(void const *argument){
	int8_t volume = 0;
	bool s1_press, s2_press;
	
	buzzer_per_set(0x1FFF);	
	while(1){
		osSignalWait(0x0001, osWaitForever);
		s1_press = button_read_s1();
		s2_press = button_read_s2();
		//XNOR > 1:0 ou 0:1 retornam true
		if(s1_press != s2_press){
			if(s1_press) volume++;
			if(s2_press) volume--;
			if(volume > 9) volume = 9;
			if(volume < 0) volume = 0;
			buzzer_vol_set((volume+1)*0xFFFF/11);
			buzzer_write(true);
			osDelay(2000);
			buzzer_write(false);
		}
		signal_func(tid_accel);     
	}
}
	
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/

void t_rgb(void const *argument){
	while(1){
		osSignalWait(0x0001, osWaitForever);
	
	
	
		signal_func(tid_buzzer); 
	}
}
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
	
void t_mic(void const *argument){
	while(1){
		osSignalWait(0x0001, osWaitForever);
		//Desenho
			sRect.i16XMin = 15;
			sRect.i16YMin = 110;
			sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
			sRect.i16YMax = 128;
			GrContextForegroundSet(&sContext, ClrBlack);
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrRectFill(&sContext, &sRect);
			GrContextForegroundSet(&sContext, ClrWhite);
			
			intToString(mic_read(), pbuf, 10, 10);
			GrStringDrawCentered(&sContext,(char*)pbuf, -1,
												 GrContextDpyWidthGet(&sContext) - 100, 
			((GrContextDpyHeightGet(&sContext)- 17)) + 10,0);	
			
			
			if(mic_read()<=200)
			{
				led_on(0);
				osDelay(500);
				led_off(0);
			}
		
		signal_func(tid_rgb); 
	}		
}
	
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
	
void t_light(void const *argument){
	
while(1){
	osSignalWait(0x0001, osWaitForever);
		sRect.i16XMin = 15;
		sRect.i16YMin = 91;
		sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
		sRect.i16YMax = 110;
		GrContextForegroundSet(&sContext, ClrBlack);
		GrContextBackgroundSet(&sContext, ClrBlack);
		GrRectFill(&sContext, &sRect);
		GrContextForegroundSet(&sContext, ClrWhite);
		
		intToString(opt_read(), pbuf, 10, 10);
		GrStringDrawCentered(&sContext,(char*)pbuf, -1,
											 GrContextDpyWidthGet(&sContext) - 100, 
		((GrContextDpyHeightGet(&sContext)- 35)) + 10,0);	
		
		signal_func(tid_mic);   
	}
}
	
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
	
void t_temp(void const *argument){
	int16_t temp;
	while(1){
			osSignalWait(0x0001, osWaitForever);
				sRect.i16XMin = 15;
					sRect.i16YMin = 70;
					sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
					sRect.i16YMax = 91;
					temp = temp_read()/32;
					intToString(temp, pbuf, 10, 10);
					GrContextForegroundSet(&sContext, ClrBlack);
					GrContextBackgroundSet(&sContext, ClrBlack);
					GrRectFill(&sContext, &sRect);

					GrContextForegroundSet(&sContext, ClrWhite);
					GrStringDrawCentered(&sContext,(char*)pbuf , -1,
														 GrContextDpyWidthGet(&sContext) - 100, 
					((GrContextDpyHeightGet(&sContext)- 55)) + 10,0);	
		
		signal_func(tid_light);   
	}
}
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/

void t_servo(void const *argument){
	float joy_reading;
	while(1){
		osSignalWait(0x0001, osWaitForever);
		joy_reading = joy_read_norm_y();
		servo_write(joy_reading*0xFFFF);
		signal_func(tid_temp); 
	}	
}
/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
	
void t_accel(void const *argument){					
	while(1){
		osSignalWait(0x0001, osWaitForever);
		sRect.i16XMin = 15;
		sRect.i16YMin = 53;
		sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
		sRect.i16YMax = 70;
		GrContextForegroundSet(&sContext, ClrBlack);
		GrContextBackgroundSet(&sContext, ClrBlack);
		GrRectFill(&sContext, &sRect);

		
		GrContextForegroundSet(&sContext, ClrWhite);
		intToString(accel_read_x(), pbuf, 10, 10);
		GrStringDrawCentered(&sContext,(char*)pbuf, -1,
											 GrContextDpyWidthGet(&sContext) - 100, 
		((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);	
		
			intToString(accel_read_y(), pbuf, 10, 10);
		GrStringDrawCentered(&sContext,(char*)pbuf, -1,
											 GrContextDpyWidthGet(&sContext) - 70, 
		((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);	
		
		intToString(accel_read_z(), pbuf, 10, 10);
		GrStringDrawCentered(&sContext,(char*) pbuf, -1,
											 GrContextDpyWidthGet(&sContext) - 40, 
		((GrContextDpyHeightGet(&sContext)- 75)) + 10,0);	
		
		signal_func(tid_servo);   
	}
}

osThreadDef(t_buzzer, 		osPriorityNormal, 1, 0);
osThreadDef(t_rgb		, 		osPriorityNormal, 1, 0);
osThreadDef(t_mic		, 		osPriorityNormal, 1, 0);
osThreadDef(t_light	,			osPriorityNormal, 1, 0);
osThreadDef(t_temp	, 	 	osPriorityNormal, 1, 0);
osThreadDef(t_servo	,  		osPriorityNormal, 1, 0);
osThreadDef(t_accel	,  		osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
	int16_t angle = 0, inc = 1;
	osKernelInitialize();

	//Initializing all peripherals
	init_all();
	//Sidelong menu creation
	init_sidelong_menu();
	
  tid_buzzer = osThreadCreate(osThread(t_buzzer), NULL);
  tid_rgb 	 = osThreadCreate(osThread(t_rgb), 		NULL);
  tid_mic 	 = osThreadCreate(osThread(t_mic), 		NULL);
  tid_light  = osThreadCreate(osThread(t_light), 	NULL);
  tid_temp   = osThreadCreate(osThread(t_temp),  	NULL);
	tid_servo  = osThreadCreate(osThread(t_servo),  NULL);
	tid_accel  = osThreadCreate(osThread(t_accel),  NULL);
	
	osKernelStart();

	signal_func(tid_accel);         

  osDelay(osWaitForever);
  while(1);
}
