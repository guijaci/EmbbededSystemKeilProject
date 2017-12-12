//..............................................................................
// Joystick header for using joystick driver functions.
//
// Copyright (c) 2017 Allan Patrick de Souza, Guilherme Jacichen, Jessica Isoton Sampaio,
// Mariana Carrião.  All rights reserved.
// Software License Agreement
//..............................................................................
#ifndef __JOY_H__
#define __JOY_H__

//..............................................................................
//Normalize read value using ADC resolution 4096
//..............................................................................
#define joy_read_norm_x() (joy_read_x()/(float)0xFFF)
#define joy_read_norm_y() (joy_read_y()/(float)0xFFF)

extern void joy_init(void);
extern uint16_t joy_read_x(void);
extern uint16_t joy_read_y(void);
extern bool joy_read_center();

#endif // __JOY_H__
