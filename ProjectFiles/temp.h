//

#ifndef __TEMP_H__
#define __TEMP_H__

#define temp_read_celsius() (temp_read()/32.0)

extern void temp_init(void);
extern int16_t temp_read(void);
extern int16_t temp_read_voltage(void);

#endif //__TEMP_H__