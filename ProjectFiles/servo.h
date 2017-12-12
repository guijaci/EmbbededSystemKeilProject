//..............................................................................
//Accelerometer header for using servo motor driver functions.
//
// Copyright (c) 2017 Allan Patrick de Souza, Guilherme Jacichen, Jessica Isoton Sampaio,
// Mariana Carrião.  All rights reserved.
// Software License Agreement
//..............................................................................

#ifndef __SERVO_H__
#define __SERVO_H__

#define PI 3.14159265359

/*******************************************************************************
 * @brief Modifies the value of the duty cycle of the Servo Motor's PWM.
 * @param  angle is the desired angle in Degrees
 *******************************************************************************/
#define servo_write_degree(angle)	servo_write((uint16_t)(((angle) +  90.0) / 180.0 * 0xFFFF))

/*******************************************************************************
 * @brief Modifies the value of the duty cycle of the Servo Motor's PWM.
 * @param angle is the desired angle in Radians
 *******************************************************************************/
#define servo_write_rad(angle)		servo_write((uint16_t)(((angle) +  PI/2) / PI 	 * 0xFFFF))

extern void servo_write(uint16_t angle);
extern void servo_init();

#endif //__SERVO_H__
