//

#ifndef __SERVO_H__
#define __SERVO_H__

#define PI 3.14159265359

#define servo_write_degree(angle)	servo_write((uint16_t)(((angle) +  90.0) / 180.0 * 0xFFFF))
#define servo_write_rad(angle)		servo_write((uint16_t)(((angle) +  PI/2) / PI 	 * 0xFFFF))

extern void servo_write(uint16_t angle);
extern void servo_init();

#endif //__SERVO_H__
