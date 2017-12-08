//..............................................................................
// Microphone header for using microphone driver functions.
//
// Copyright (c) 2017 Allan Patrick de Souza, Guilherme Jacichen, Jessica Isoton Sampaio,
// Mariana Carrião.  All rights reserved.
// Software License Agreement
//..............................................................................
#ifndef __MIC_H__
#define __MIC_H__

//..............................................................................
//Normalize read value using ADC resolution 4096
//..............................................................................
#define mic_norm(v) (mic_read(v)/(float)0xFFF)

extern void mic_init(void);
extern uint16_t mic_read(void);

#endif //__MIC_H__