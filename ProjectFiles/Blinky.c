/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    BLinky.c
 *      Purpose: RTX example program
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2014 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "cmsis_os.h"
#include "TM4C129.h"
#include "system_TM4C129.h"
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "grlib/grlib.h"
#include "C:\ti\TivaWare_C_Series-2.1.4.178\utils\uartstdio.h"
#include "cfaf128x128x16.h"
#include "math.h"

static void intToString(int64_t value, char* pBuf, uint32_t len, uint32_t base)
{
    static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
    int pos = 0;
    uint32_t tmpValue = value;

    // the buffer must not be null and at least have a length of 2 to handle one
    // digit and null-terminator
    if (pBuf == NULL || len < 2)
    {
        return;
    }

    // a valid base cannot be less than 2 or larger than 36
    // a base value of 2 means binary representation. A value of 1 would mean only zeros
    // a base larger than 36 can only be used if a larger alphabet were used.
    if (base < 2 || base > 36)
    {
        return;
    }

    // negative value
    if (value < 0)
    {
        tmpValue = -tmpValue;
        value    = -value;
        pBuf[pos++] = '-';
    }

    // calculate the required length of the buffer
    do {
        pos++;
        tmpValue /= base;
    } while(tmpValue > 0);


    if (pos > len)
    {
        // the len parameter is invalid.
        return;
    }

    pBuf[pos] = '\0';

    do {
        pBuf[--pos] = pAscii[value % base];
        value /= base;
    } while(value > 0);

    return;

}

int main (void) {
		//Impressao dos valores do ADC
		tContext sContext;
		tRectangle sRect;
			char pBuf[10];
	uint32_t ui32SysClock;
	
int32_t i32Val;
	
			ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL |
                                       SYSCTL_CFG_VCO_480), 120000000);
//
// Enable the GPIOA peripheral
//
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
//
// Wait for the GPIOA module to be ready.
//
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
{
}
//
// Register the port-level interrupt handler. This handler is the first
// level interrupt handler for all the pin interrupts.
//

GPIOIntRegister(GPIO_PORTE_BASE, GPIOE_Handler);
//
// Initialize the GPIO pin configuration.
//
// Set pins 2, 4, and 5 as input, SW controlled.
//
GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
GPIO_PIN_3);
//
// Set pins 0 and 3 as output, SW controlled.
//
//GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_3);
//
// Make pins 2 and 4 rising edge triggered interrupts.
//
GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_RISING_EDGE);
//
// Make pin 5 high level triggered interrupts.
//
//GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_HIGH_LEVEL);
//
// Read some pins.
//
i32Val = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3);
//
// Write some pins. Even though pins 2, 4, and 5 are specified, those pins
// are unaffected by this write because they are configured as inputs. At
// the end of this write, pin 0 is low, and pin 3 is high.
//
//GPIOPinWrite(GPIO_PORTA_BASE,
//(GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 |
//GPIO_PIN_4 | GPIO_PIN_5),
//(GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
//GPIO_PIN_7));
//
// Enable the pin interrupts.
//
GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_3);

while(1)
{

i32Val = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3);
	
		sRect.i16XMin = 0;
			sRect.i16YMin = 0;
			sRect.i16XMax = GrContextDpyWidthGet(&sContext) -1;
			sRect.i16YMax = GrContextDpyHeightGet(&sContext) -1;
			GrContextForegroundSet(&sContext, ClrBlack);
			GrRectFill(&sContext, &sRect);
				
			GrContextForegroundSet(&sContext, ClrWhite);
			GrRectDraw(&sContext, &sRect);

			
			intToString(i32Val/*converted_Value*/, pBuf, 10, 10);		
			
			//VREFP - VREFN = 3.3
	
GrStringDrawCentered(&sContext, pBuf, 10,
											 GrContextDpyWidthGet(&sContext) / 2,
											 ((GrContextDpyHeightGet(&sContext) - 84) / 2) + 24,
											 0);
intToString(i32Val/*converted_Value*/, pBuf, 10, 10);		

		SysCtlDelay(ui32SysClock / 12);
}

}
