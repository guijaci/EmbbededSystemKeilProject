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
#define normalize(v) (v/(float)0xFFF)
	
//..............................................................................
//Covert value to voltage using Vref 3.3V
//..............................................................................	
#define to_voltage(v) (normalize(v)*3.3)

extern void accel_init(void);
extern uint32_t accel_read_x(void);
extern uint32_t accel_read_y(void);
extern uint32_t accel_read_z(void);

#endif

