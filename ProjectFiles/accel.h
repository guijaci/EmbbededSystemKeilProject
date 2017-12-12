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
#define accel_read_norm_x() (accel_read_x()/(float)0xFFF)
#define accel_read_norm_y() (accel_read_y()/(float)0xFFF)
#define accel_read_norm_z() (accel_read_z()/(float)0xFFF)
	
//..............................................................................
//Covert value to voltage using Vref 3.3V
//..............................................................................	
#define accel_read_voltage_x() (accel_read_norm_x()*3.3)
#define accel_read_voltage_y() (accel_read_norm_y()*3.3)
#define accel_read_voltage_z() (accel_read_norm_z()*3.3)


extern void accel_init(void);
extern uint16_t accel_read_x(void);
extern uint16_t accel_read_y(void);
extern uint16_t accel_read_z(void);

#endif // __ACCEL_H__

