//..............................................................................
// Microphone header for using microphone driver functions.
//
// Allan Patrick de Souza - <allansouza@alunos.utfpr.edu.br>
// Guilherme Jacichen     - <jacichen@alunos.utfpr.edu.br>
// Jessica Isoton Sampaio - <jessicasampaio@alunos.utfpr.edu.br>
// Mariana Carrião        - <mcarriao@alunos.utfpr.edu.br>
//
// All rights reserved.
// Software License Agreement
//..............................................................................
#ifndef __MIC_H__
#define __MIC_H__

//..............................................................................
//Normalize read value using ADC resolution 4096
//..............................................................................
#define mic_norm() (mic_read()/(float)0xFFF)

extern void mic_init(void);
extern uint16_t mic_read(void);

#endif //__MIC_H__