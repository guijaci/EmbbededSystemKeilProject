//

#ifndef __SERVO_H__
#define __SERVO_H__

#define PI 3.14159265359

#define servo_write_degree90(angle)		servo_write((uint16_t)((angle +  45.0) /  90.0 * 0xFFFF))
#define servo_write_degree180(angle)	servo_write((uint16_t)((angle +  90.0) / 180.0 * 0xFFFF))
#define servo_write_degree360(angle)	servo_write((uint16_t)((angle + 180.0) / 360.0 * 0xFFFF))

#define servo_write_rad90(angle)			servo_write((uint16_t)((angle +  PI/4)*2 / PI * 0xFFFF))
#define servo_write_rad180(angle)			servo_write((uint16_t)((angle +  PI/2)   / PI * 0xFFFF))
#define servo_write_rad360(angle)			servo_write((uint16_t)((angle +  PI  )/2 / PI * 0xFFFF))

extern void servo_write(uint16_t angle);
extern void servo_init();

#endif //__SERVO_H__
