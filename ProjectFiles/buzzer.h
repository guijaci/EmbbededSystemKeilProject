#ifndef __BUZZER_H__
#define __BUZZER_H__

typedef enum {FREQ_0, FREQ_1, FREQ_2, FREQ_3, FREQ_4, FREQ_5, FREQ_6} 
freq_t;

extern void buzzer_init();
extern void buzzer_read();
extern void buzzer_frequency_set(freq_t);
extern uint32_t buzzer_frequency_get();

#endif //__BUZZER_H