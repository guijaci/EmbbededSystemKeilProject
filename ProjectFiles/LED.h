/*----------------------------------------------------------------------------
 * Name:    LED.h
 * Purpose: low level LED definitions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2014 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#ifndef LED_H
#define LED_H

/* LED Definitions */
#define LED_NUM     4                        /* Number of user LEDs          */

extern void led_init(void);
extern void led_on        (uint8_t num);
extern void led_off       (uint8_t num);
extern void led_write			(uint8_t value);

#endif
