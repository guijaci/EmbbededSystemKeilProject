//..............................................................................
//Speaker header for using speaker driver functions.
//
// Copyright (c) 2017 Allan Patrick de Souza, Guilherme Jacichen, Jessica Isoton Sampaio,
// Mariana Carrião.  All rights reserved.
// Software License Agreement
//..............................................................................
#ifndef __SPEAKER_H__
#define __SPEAKER_H__

//..............................................................................
//Normalize read value using ADC resolution 4096
//..............................................................................
#define normalize(v) (v/(float)0xFFF)


extern void speaker_init(void);
extern uint32_t speaker_read(void);


#endif