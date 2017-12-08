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
#define normalize(v) (v/(float)0xFFF)


extern void joystick_init(void);
extern uint32_t joystick_read_x(void);
extern uint32_t joystick_read_y(void);


#endif
