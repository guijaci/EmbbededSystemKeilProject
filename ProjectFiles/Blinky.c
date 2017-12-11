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
#include <stdbool.h>
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

#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    7

#define sqr(a) (a*a)

osThreadId tid_buzzer;                 /* Thread id of thread: buzzer     			 	*/
osThreadId tid_servo;                  /* Thread id of thread: motor       				*/
osThreadId tid_rgb;                    /* Thread id of thread: rgb 	       				*/
osThreadId tid_accel;                  /* Thread id of thread: accelerometer      */
osThreadId tid_temp;                   /* Thread id of thread: temperture	        */
osThreadId tid_light;                  /* Thread id of thread: microphone		      */
osThreadId tid_mic;   

osMutexId mid_display;
osMutexId mid_adc;
osMutexId mid_i2c;

osMailQId mqid_accel_to_rgb;
osMailQId mqid_joy_to_rgb;
osMailQId mqid_joy_to_servo;

//To print on the screen
tContext sContext;

//Tipos para MailQueue
typedef struct{
	uint16_t x, y, z;
} vector3d_ui16_t;

typedef struct{
	uint16_t x, y;
} vector2d_ui16_t;

typedef struct{
	vector2d_ui16_t dirs;
	bool center;
} joy_reading_t;

/*----------------------------------------------------------------------------
 *  Transforming int to string
 *---------------------------------------------------------------------------*/
static void intToString(int64_t value, char * pBuf, uint32_t len, uint32_t base, uint8_t zeros){
	static const char* pAscii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	bool n = false;
	int pos = 0, d = 0;
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

	if (zeros > len)
		return;
	
	// negative value
	if (value < 0)
	{
			tmpValue = -tmpValue;
			value    = -value;
			pBuf[pos++] = '-';
			n = true;
	}

	// calculate the required length of the buffer
	do {
			pos++;
			tmpValue /= base;
	} while(tmpValue > 0);


	if (pos > len)
			// the len parameter is invalid.
			return;

	if(zeros > pos){
		pBuf[zeros] = '\0';
		do{
			pBuf[d++ + (n ? 1 : 0)] = pAscii[0]; 
		}
		while(zeros > d + pos);
	}
	else
		pBuf[pos] = '\0';

	pos += d;
	do {
			pBuf[--pos] = pAscii[value % base];
			value /= base;
	} while(value > 0);
}

static void floatToString(float value, char *pBuf, uint32_t len, uint32_t base, uint8_t zeros, uint8_t precision){
	static const char* pAscii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint8_t start = 0xFF;
	if(len < 2)
		return;
	
	if (base < 2 || base > 36)
		return;
	
	if(zeros + precision + 1 > len)
		return;
	
	intToString((int64_t) value, pBuf, len, base, zeros);
	while(pBuf[++start] != '\0' && start < len); 

	if(start + precision + 1 > len)
		return;
	
	pBuf[start+precision+1] = '\0';
	
	if(value < 0)
		value = -value;
	pBuf[start++] = '.';
	while(precision-- > 0){
		value -= (uint32_t) value;
		value *= (float) base;
		pBuf[start++] = pAscii[(uint32_t) value];
	}
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

void draw_circle(uint16_t x, uint16_t y){
	GrCircleDraw(&sContext, 
		(sContext.psFont->ui8MaxWidth)*x + (sContext.psFont->ui8MaxWidth)/2, 
		(sContext.psFont->ui8Height+2)*y + sContext.psFont->ui8Height/2 - 1, 
		(sContext.psFont->ui8MaxWidth)/2);
}

void fill_circle(uint16_t x, uint16_t y){
	GrCircleFill(&sContext, 
		(sContext.psFont->ui8MaxWidth)*x + sContext.psFont->ui8MaxWidth/2, 
		(sContext.psFont->ui8Height+2)*y + sContext.psFont->ui8Height/2 - 1, 
		(sContext.psFont->ui8MaxWidth)/2-1);
}

/*----------------------------------------------------------------------------
 *    Sidelong menu with thread's name
 *---------------------------------------------------------------------------*/
void init_sidelong_menu(){
	uint8_t i;
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrWhite);
	GrContextBackgroundSet(&sContext, ClrBlack);
	
	//Escreve menu lateral:
	GrStringDraw(&sContext,"BUZ", -1, 0, (sContext.psFont->ui8Height+2)*0, true);
	GrStringDraw(&sContext,"SRV", -1, 0, (sContext.psFont->ui8Height+2)*1, true);
	GrStringDraw(&sContext,"RGB", -1, 0, (sContext.psFont->ui8Height+2)*2, true);
	GrStringDraw(&sContext,"ACC", -1, 0, (sContext.psFont->ui8Height+2)*3, true);
	GrStringDraw(&sContext,"TMP", -1, 0, (sContext.psFont->ui8Height+2)*4, true);
	GrStringDraw(&sContext,"OPT", -1, 0, (sContext.psFont->ui8Height+2)*5, true);
	GrStringDraw(&sContext,"MIC", -1, 0, (sContext.psFont->ui8Height+2)*6, true);
	GrStringDraw(&sContext,"JOY", -1, 0, (sContext.psFont->ui8Height+2)*7, true);

	for(i = 0; i < 8; i++)
		draw_circle(4, i);
}
	
uint32_t saturate(uint8_t r, uint8_t g, uint8_t b){
	uint8_t *max = &r, 
					*mid = &g, 
					*min = &b,
					*aux, 
					div, num;
	if (*mid > *max){ aux = max; max = mid; mid = aux; }
	if (*min > *mid){ aux = mid; mid = min; min = aux; }
	if (*mid > *max){	aux = max; max = mid; mid = aux; }
	if(*max != *min){
		div = *max-*min;
		num = *mid-*min;
		*max = 0xFF;
		*min = 0x00;
		*mid = (uint8_t) num*0xFF/div;
	}
	return 	(((uint32_t) r) << 16) | 
					(((uint32_t) g) <<  8) | 
					( (uint32_t) b       );
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
 *      Function 'set_thread_status'
*			status: 1 for Running, and 0 for Waiting
 *---------------------------------------------------------------------------*/
void thread_status(uint8_t n, bool status)  {
	osMutexWait (mid_display, osWaitForever);
		GrContextForegroundSet(&sContext, status ? ClrLightGreen : ClrRed);
		fill_circle(4,n);
	osMutexRelease (mid_display);
}

/*----------------------------------------------------------------------------
 *      Thread 1 't_buzzer'
 *---------------------------------------------------------------------------*/
void t_buzzer(void const *argument){
	char pbuf[10];
	int8_t volume = 0;
	bool s1_press, s2_press;
	
	buzzer_per_set(0x03FF);	
	
	osMutexWait (mid_display, osWaitForever);
		GrContextBackgroundSet(&sContext, ClrBlack);
		draw_circle(10, 0);
		draw_circle(13, 0);
	osMutexRelease (mid_display);
	while(1){
		thread_status(0, true);
		//osSignalWait(0x0001, osWaitForever);
		s1_press = button_read_s1();
		s2_press = button_read_s2();
		//XNOR > 1:0 ou 0:1 retornam true
		if(s1_press != s2_press){
			if(s1_press) volume++;
			if(s2_press) volume--;
			if(volume > 9) volume = 9;
			if(volume < 0) volume = 0;
			buzzer_vol_set((volume+1)*0x0FFF/11);
		}
			
		intToString(volume+1, pbuf, 10, 10, 2); 

		osMutexWait (mid_display, osWaitForever);
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbuf, -1, 
				(sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*0, true);
			GrContextForegroundSet(&sContext, s1_press ? ClrWhite : ClrBlack);
			fill_circle(13, 0);
			GrContextForegroundSet(&sContext, s2_press ? ClrWhite : ClrBlack);
			fill_circle(10, 0);
		osMutexRelease (mid_display);

		if(s1_press != s2_press){
			buzzer_write(true);
			osDelay(2000);
			buzzer_write(false);			
		}
		
		thread_status(0, false);
		osDelay(20);
	}
}

void t_rgb(void const *argument){
	char pbufx[10], pbufy[10], pbufz[10];
	bool 	enable = false, 
				center = false,
				pushed = false, 
				rising = false;
	float intensity = 0;
	uint16_t 	x 	= 0, 
						y 	= 0, 
						z 	= 0,
						joy = 0;
	uint8_t  	r, g, b;
	uint32_t color;
	osEvent evt;
	vector3d_ui16_t *mail_msg_accel;
	joy_reading_t 	*mail_msg_joy;
	tRectangle rect;
	rect.i16XMin = (sContext.psFont->ui8MaxWidth)*20+1;
	rect.i16XMax = (sContext.psFont->ui8MaxWidth)*21-1;
	rect.i16YMin = (sContext.psFont->ui8Height+2)* 2+1;
	rect.i16YMax = (sContext.psFont->ui8Height+2)* 3-5;
	
	while(1){
		evt = osMailGet(mqid_accel_to_rgb, 0);
		if(evt.status == osEventMail){
			mail_msg_accel = (vector3d_ui16_t*) evt.value.p;
				x = mail_msg_accel->x;
				y = mail_msg_accel->y;
				z = mail_msg_accel->z;
			osMailFree(mqid_accel_to_rgb, mail_msg_accel);
		}

		evt = osMailGet(mqid_joy_to_rgb, 0);
		if(evt.status == osEventMail){
			mail_msg_joy = (joy_reading_t*) evt.value.p;
				center 	= mail_msg_joy->center;
				joy 		= mail_msg_joy->dirs.x;
			osMailFree(mqid_joy_to_rgb, mail_msg_joy);
		}
		thread_status(2, true);

		rising = !pushed && center;
		pushed = center;
		if(rising)
			enable = !enable;
			
		intensity = enable ? joy/(float)0xFFF : intensity;
		
		r = x*0xFF/0xFFF;
		g = y*0xFF/0xFFF;
		b = z*0xFF/0xFFF;
		
		color = saturate(r,g,b);
		color = rgb_color_intensity(color, intensity);
		rgb_write_color(color);
		
		intToString((int32_t)(rgb_color_r(color)*100/0xFF), pbufx, 10, 10, 3);
		intToString((int32_t)(rgb_color_g(color)*100/0xFF), pbufy, 10, 10, 3);		
		intToString((int32_t)(rgb_color_b(color)*100/0xFF), pbufz, 10, 10, 3);		
		
		osMutexWait (mid_display, osWaitForever);
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrRed);
			GrStringDraw(&sContext,(char*)pbufx, -1, 
				(sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*2, true);
			GrContextForegroundSet(&sContext, ClrLightGreen);
			GrStringDraw(&sContext,(char*)pbufy, -1, 
				(sContext.psFont->ui8MaxWidth)*11, (sContext.psFont->ui8Height+2)*2, true);
			GrContextForegroundSet(&sContext, ClrBlue);
			GrStringDraw(&sContext,(char*)pbufz, -1, 
				(sContext.psFont->ui8MaxWidth)*16, (sContext.psFont->ui8Height+2)*2, true);
			GrContextForegroundSet(&sContext, color);
			GrRectFill(&sContext, &rect);
		osMutexRelease (mid_display);

		thread_status(2, false);
		osDelay(20);
	}
}

void t_mic(void const *argument){
	char pbuf[10];
	float mic, sqr;
	while(1){
		thread_status(6, true);
		//Desenho
		osMutexWait (mid_adc, osWaitForever);
			mic = mic_norm();
		osMutexRelease (mid_adc);
		
		intToString((int32_t) (mic*200-100), pbuf, 10, 10, 4);

		sqr = (mic-.5)*2;
		sqr = sqr(sqr);
		
		if(sqr > sqr(.2)) led_on (LED_A);
		else 							led_off(LED_A);
		if(sqr > sqr(.4)) led_on (LED_B);
		else 							led_off(LED_B);
		if(sqr > sqr(.6)) led_on (LED_C);
		else 							led_off(LED_C);
		if(sqr > sqr(.8)) led_on (LED_D);
		else 							led_off(LED_D);

		osMutexWait (mid_display, osWaitForever);
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbuf, -1, 
				(sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*6, true);
		osMutexRelease (mid_display);
		
		thread_status(6, false);
		osDelay(20);
	}		
}
	
void t_light(void const *argument){
	char pbuf[10];
	float lux;
	
	while(1){
		thread_status(5, true);
		
		osMutexWait (mid_i2c, osWaitForever);
			lux = opt_fread_lux();
		osMutexRelease (mid_i2c);
		
		floatToString(lux, pbuf, 10, 10, 4, 3);
		
		osMutexWait (mid_display, osWaitForever);
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbuf, -1, 
				(sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*5, true);
		osMutexRelease (mid_display);
		
		thread_status(5, false);
		osDelay(20);
	}
}
	
void t_temp(void const *argument){
	char pbuf[10];
	float temp;

	while(1){	
		thread_status(4, true);
		
		osMutexWait (mid_i2c, osWaitForever);
			temp = temp_read_celsius();
		osMutexRelease (mid_i2c);

		floatToString(temp, pbuf, 10, 10, 2, 3);
		
		osMutexWait (mid_display, osWaitForever);
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbuf, -1, 
				(sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*4, true);
		osMutexRelease (mid_display);

		thread_status(4, false);
		osDelay(20);
	}
}

void t_servo(void const *argument){
	char pbuf[10];
	uint16_t joy;
	joy_reading_t 	*mail_msg_joy;
	osEvent evt;
	
	while(1){
		evt = osMailGet(mqid_joy_to_servo, osWaitForever);
		if(evt.status == osEventMail){
			mail_msg_joy = (joy_reading_t*) evt.value.p;
				joy 		= mail_msg_joy->dirs.y;
			osMailFree(mqid_joy_to_servo, mail_msg_joy);
		}
		thread_status(1, true);

		servo_write(joy*0xFFFF/0xFFF);
		intToString(joy*180/0xFFF-90, pbuf, 10, 10, 3);
		
		osMutexWait (mid_display, osWaitForever);
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbuf, -1, 
				(sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*1, true);
		osMutexRelease (mid_display);

		thread_status(1, false);
	}	
}

void t_accel(void const *argument){					
	char pbufx[10], pbufy[10], pbufz[10];
	uint16_t x, y, z;
	vector3d_ui16_t* mail_msg;

	while(1){
		thread_status(3, true);
		osMutexWait (mid_adc, osWaitForever);
			x = accel_read_x();
			y = accel_read_y();
			z = accel_read_z();
		osMutexRelease (mid_adc);

		floatToString((x*3.30/0xFFF), pbufx, 10, 10, 1, 2);
		floatToString((y*3.30/0xFFF), pbufy, 10, 10, 1, 2);		
		floatToString((z*3.30/0xFFF), pbufz, 10, 10, 1, 2);
			
		osMutexWait (mid_display, osWaitForever);
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrMagenta);
			GrStringDraw(&sContext,(char*)pbufx, -1, 
				(sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*3, true);
			GrContextForegroundSet(&sContext, ClrYellow);
			GrStringDraw(&sContext,(char*)pbufy, -1, 
				(sContext.psFont->ui8MaxWidth)*11, (sContext.psFont->ui8Height+2)*3, true);
			GrContextForegroundSet(&sContext, ClrCyan);
			GrStringDraw(&sContext,(char*)pbufz, -1, 
				(sContext.psFont->ui8MaxWidth)*16, (sContext.psFont->ui8Height+2)*3, true);
		osMutexRelease (mid_display);
		
		mail_msg = (vector3d_ui16_t*) osMailAlloc(mqid_accel_to_rgb, osWaitForever);
			mail_msg->x = x;
			mail_msg->y = y;
			mail_msg->z = z;
		osMailPut(mqid_accel_to_rgb, mail_msg);

		thread_status(3, false);
		osDelay(20);
	}
}

void t_joy(){
	char pbufx[10], pbufy[10];
	uint16_t x, y;
	bool center;
	joy_reading_t *mail_msg;

	osMutexWait (mid_display, osWaitForever);
		GrContextForegroundSet(&sContext, ClrWhite);
		draw_circle(16, 7);
	osMutexRelease (mid_display);
	while(1){
		thread_status(7, true);
		
		osMutexWait (mid_adc, osWaitForever);
			x = joy_read_x();
			y = joy_read_y();
		osMutexRelease (mid_adc);		
		center = joy_read_center();

		intToString(x*200/0xFFF-100, pbufx, 10, 10, 4);
		intToString(y*200/0xFFF-100, pbufy, 10, 10, 4);
			
		osMutexWait (mid_display, osWaitForever);
			GrContextBackgroundSet(&sContext, ClrBlack);
			GrContextForegroundSet(&sContext, ClrWhite);
			GrStringDraw(&sContext,(char*)pbufx, -1, 
				(sContext.psFont->ui8MaxWidth)*6,  (sContext.psFont->ui8Height+2)*7, true);
			GrStringDraw(&sContext,(char*)pbufy, -1, 
				(sContext.psFont->ui8MaxWidth)*11, (sContext.psFont->ui8Height+2)*7, true);
			GrContextForegroundSet(&sContext, center ? ClrWhite : ClrBlack);
			fill_circle(16, 7);
		osMutexRelease (mid_display);

		mail_msg = (joy_reading_t*) osMailAlloc(mqid_joy_to_rgb, osWaitForever);
			mail_msg->dirs.x = x;
			mail_msg->dirs.y = y;
			mail_msg->center = center;
		osMailPut(mqid_joy_to_rgb, mail_msg);

		mail_msg = (joy_reading_t*) osMailAlloc(mqid_joy_to_servo, osWaitForever);
			mail_msg->dirs.x = x;
			mail_msg->dirs.y = y;
			mail_msg->center = center;
		osMailPut(mqid_joy_to_servo, mail_msg);

		thread_status(7, false);
		osDelay(20);
	}	
}

osThreadDef (t_buzzer, 		osPriorityNormal, 1, 0);
osThreadDef (t_rgb,	 			osPriorityNormal, 1, 0);
osThreadDef (t_mic, 			osPriorityNormal, 1, 0);
osThreadDef (t_light,			osPriorityNormal, 1, 0);
osThreadDef (t_temp, 	 		osPriorityNormal, 1, 0);
osThreadDef (t_servo,  		osPriorityNormal, 1, 0);
osThreadDef (t_accel,  		osPriorityNormal, 1, 0);

osMutexDef (m_display	);
osMutexDef (m_adc			);
osMutexDef (m_i2c			);

osMailQDef (mq_accel_to_rgb, 	16, vector3d_ui16_t);
osMailQDef (mq_joy_to_rgb, 		16, joy_reading_t);
osMailQDef (mq_joy_to_servo, 	16, joy_reading_t);

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
	
  tid_buzzer 	= osThreadCreate (osThread(t_buzzer),	NULL);
  tid_rgb 	 	= osThreadCreate (osThread(t_rgb), 		NULL);
  tid_mic 	 	= osThreadCreate (osThread(t_mic), 		NULL);
  tid_light  	= osThreadCreate (osThread(t_light), 	NULL);
  tid_temp   	= osThreadCreate (osThread(t_temp),  	NULL);
	tid_servo  	= osThreadCreate (osThread(t_servo),  NULL);
	tid_accel  	= osThreadCreate (osThread(t_accel),  NULL);

	mid_display = osMutexCreate (osMutex (m_display));
	mid_adc 		= osMutexCreate (osMutex (m_adc));
	mid_i2c 		= osMutexCreate (osMutex (m_i2c));
	
	mqid_accel_to_rgb	= osMailCreate (osMailQ(mq_accel_to_rgb), 	NULL);
	mqid_joy_to_rgb		= osMailCreate (osMailQ(mq_joy_to_rgb), 		NULL);
	mqid_joy_to_servo	= osMailCreate (osMailQ(mq_joy_to_servo), 	NULL);
	
	osKernelStart();

	t_joy();

  osDelay(osWaitForever);
  while(1);
}
