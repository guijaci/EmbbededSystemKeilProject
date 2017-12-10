#ifndef __BUZZER_H__
#define __BUZZER_H__

extern void buzzer_init();
extern void buzzer_write(bool);
extern void buzzer_vol_set(uint16_t);
extern void buzzer_per_set(uint16_t);

#endif //__BUZZER_H