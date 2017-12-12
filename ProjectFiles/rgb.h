//..............................................................................
//Accelerometer header for using LED RGB driver functions.
//
// Copyright (c) 2017 Allan Patrick de Souza, Guilherme Jacichen, Jessica Isoton Sampaio,
// Mariana Carrião.  All rights reserved.
// Software License Agreement
//..............................................................................

#ifndef __RGB_H__
#define __RGB_H__

// Colors definitions
//												  33333333 22222222 11111111 00000000
// Color 24 bits:					  -------- RRRRRRRR GGGGGGGG BBBBBBBB
#define RGB_OFF						0x00000000
#define RGB_WHITE					0x00FFFFFF
#define RGB_RED						0x00FF0000
#define RGB_GREEN					0x0000FF00
#define RGB_BLUE					0x000000FF
#define RGB_YELLOW				0x00FFFF00
#define RGB_CYAN					0x0000FFFF
#define RGB_MAGENTA				0x00FF00FF

/******************************************************************************
 * @brief Gets the red value of a color int 24bits format.
 * @param  c is the color 24bits to extract the red value
 * @return the 8bit red color component value
 ******************************************************************************/
#define rgb_color_r(c)	(((c) & 0x00FF0000) >> 16 )

/******************************************************************************
 * @brief Gets the green value of a color int 24bits format.
 * @param  c is the color 24bits to extract the red value
 * @return the 8bit green color component value
 ******************************************************************************/
#define rgb_color_g(c)  (((c) & 0x0000FF00) >>  8 )

/******************************************************************************
 * @brief Gets the blue value of a color int 24bits format.
 * @param  c is the color 24bits to extract the red value
 * @return the 8bit blue color component value
 ******************************************************************************/
#define rgb_color_b(c)	( (c) & 0x000000FF        )


/******************************************************************************
 * @brief Modifies a color in 24bits format with an intensity multiplier.
 * @param  c is the color 24bits to extract the red value
 * @param  i is the intensity multiplier in floating point, values lesser than 
 *				 1 should decrease the intensity, where values greater than 1 should
 *				 increase it.
 * @return the color with the applied intensity multplier
 ******************************************************************************/
#define rgb_color_intensity(c, i)														\
		((((uint8_t) (rgb_color_r(c) * (i))) & 0xFF) << 16)	|		\
		((((uint8_t) (rgb_color_g(c) * (i))) & 0xFF) << 8 )	|		\
		 (((uint8_t) (rgb_color_b(c) * (i))) & 0xFF)						

extern void rgb_write_r(uint8_t r);
extern void rgb_write_g(uint8_t g);
extern void rgb_write_b(uint8_t b);
extern void rgb_write(uint8_t r, uint8_t g, uint8_t b);
extern void rgb_write_color(uint32_t rgb);
extern void rgb_init(void);

#endif //__RGB_H__