//

#ifndef __RGB_H__
#define __RGB_H__

//												3333 2222 1111 0000
//Color 24 bits:					---- RRRR GGGG BBBB
#define RGB_OFF						0x00000000
#define RGB_WHITE					0x00FFFFFF
#define RGB_RED						0x00FF0000
#define RGB_GREEN					0x0000FF00
#define RGB_BLUE					0x000000FF
#define RGB_YELLOW				0x00FFFF00
#define RGB_CYAN					0x0000FFFF
#define RGB_MAGENTA				0x00FF00FF

#define GetR(c)	((c & 0x00FF0000) >> 16 )
#define GetG(c) ((c & 0x0000FF00) >>  8 )
#define GetB(c)	( c & 0x000000FF        )

#define ApplyLightness(c, i)												\
		((((uint8_t) (GetR(c) * i)) & 0xFF) << 16)	|		\
		((((uint8_t) (GetG(c) * i)) & 0xFF) << 8 )	|		\
		 (((uint8_t) (GetB(c) * i)) & 0xFF)						

extern void rgb_write_r(uint8_t r);
extern void rgb_write_g(uint8_t g);
extern void rgb_write_b(uint8_t b);
extern void rgb_write(uint8_t r, uint8_t g, uint8_t b);
extern void rgb_write_color(uint32_t rgb);
extern void rgb_init(void);

#endif //__RGB_H__