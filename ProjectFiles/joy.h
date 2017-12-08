//..............................................................................
//Joystick header for using joystick driver functions.
//
// Copyright (c) 2017 Allan Patrick de Souza, Guilherme Jacichen, Jessica Isoton Sampaio,
// Mariana Carrião.  All rights reserved.
// Software License Agreement
//..............................................................................
#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

//..............................................................................
//Normalize read value using ADC resolution 4096
//..............................................................................
#define joy_read_norm_x(v) (joy_read_x(v)/(float)0xFFF)
#define joy_read_norm_y(v) (joy_read_y(v)/(float)0xFFF)

extern void joy_init(void);
extern uint32_t joy_read_x(void);
extern uint32_t joy_read_y(void);

#endif
