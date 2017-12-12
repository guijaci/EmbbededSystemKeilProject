//..............................................................................
// temp.h - header for using OPT3001 driver functions .
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


#ifndef __OPT_H__
#define __OPT_H__

#define opt_fread_lux() (opt_read_lux()/ (float) (1<<6))

extern void  opt_init();
extern uint16_t opt_read();
extern uint32_t opt_read_lux();

#endif //__OPT_H__