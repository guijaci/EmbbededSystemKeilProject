//..............................................................................
//Accelerometer header for using accelerometer driver functions.
//
// Copyright (c) 2017 Allan Patrick de Souza, Guilherme Jacichen, Jessica Isoton Sampaio,
// Mariana Carrião.  All rights reserved.
// Software License Agreement
//..............................................................................
#ifndef __ACCEL_H__
#define __ACCEL_H__

//..............................................................................
//Normalize read value using ADC resolution 4096
//..............................................................................
#define accel_read_norm_x(v) (accel_read_x(v)/(float)0xFFF)
#define accel_read_norm_y(v) (accel_read_y(v)/(float)0xFFF)
#define accel_read_norm_z(v) (accel_read_z(v)/(float)0xFFF)
	
//..............................................................................
//Covert value to voltage using Vref 3.3V
//..............................................................................	
#define accel_read_voltage_x(v) (accel_read_norm_x(v)*3.3)
#define accel_read_voltage_y(v) (accel_read_norm_y(v)*3.3)
#define accel_read_voltage_z(v) (accel_read_norm_z(v)*3.3)


extern void accel_init(void);
extern uint16_t accel_read_x(void);
extern uint16_t accel_read_y(void);
extern uint16_t accel_read_z(void);

#endif // __ACCEL_H__

