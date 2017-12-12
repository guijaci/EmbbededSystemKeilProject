//
//..............................................................................
// temp.h -for using TMP006 driver functions.
//
// Copyright (c) 2017 
// Allan Patrick de Souza - <allansouza@alunos.utfpr.edu.br>
// Guilherme Jacichen     - <jacichen@alunos.utfpr.edu.br>
// Jessica Isoton Sampaio - <jessicasampaio@alunos.utfpr.edu.br>
// Mariana Carrião        - <mcarriao@alunos.utfpr.edu.br>
//
// All rights reserved. 
// Software License Agreement
//...............................................................................
#ifndef __TEMP_H__
#define __TEMP_H__
//convert to celsius
#define temp_read_celsius() (temp_read()/32.0)

extern void temp_init(void);
extern int16_t temp_read(void);
extern int16_t temp_read_voltage(void);

#endif //__TEMP_H__