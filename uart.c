/*
 * uart.c
 *
 *  Created on: Jan 30, 2013
 *      Author: x0184343
 */

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "BP_HAL.h"
#include "uart.h"
#include "stdarg.h"

//*****************************************************************************
//
// The UART interrupt handler.
//
//*****************************************************************************
void
UARTIntHandler(void)
{
    unsigned long ulStatus;

    //
    // Get the interrrupt status.
    //
    ulStatus = ROM_UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART0_BASE, ulStatus);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while(ROM_UARTCharsAvail(UART0_BASE))
    {
    	 ROM_UARTCharPutNonBlocking(UART0_BASE, ROM_UARTCharGetNonBlocking(UART0_BASE));
    }
}

//*****************************************************************************
//
// Send a string to the UART.
//
//*****************************************************************************
void UARTSend(char *pucBuffer)
{
    while(*pucBuffer)
        ROM_UARTCharPutNonBlocking(UART0_BASE, *pucBuffer++);
}

inline void putc(char c)
{
	ROM_UARTCharPutNonBlocking(UART0_BASE,c);
}

inline void puts(char* s)
{
	UARTSend(s);
}

void UART_Init()
{
    	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

 	    //
 	    // Set GPIO A0 and A1 as UART pins.
	    //
	    GPIOPinConfigure(GPIO_PA0_U0RX);
	    GPIOPinConfigure(GPIO_PA1_U0TX);
	    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	    //
	    // Configure the UART for 115,200, 8-N-1 operation.
	    //
	    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
	                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
	                             UART_CONFIG_PAR_NONE));

	    //
	    // Enable the UART interrupt.
	    //
	    ROM_IntEnable(INT_UART0);
	    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

	    //
	    // Prompt for text to be entered.
	    //
	    UARTSend("[start]\n");
}

static const unsigned long dv[] = {
//  4294967296      // 32 bit unsigned max
    1000000000,     // +0
     100000000,     // +1
      10000000,     // +2
       1000000,     // +3
        100000,     // +4
//       65535      // 16 bit unsigned max
         10000,     // +5
          1000,     // +6
           100,     // +7
            10,     // +8
             1,     // +9
};

static void xtoa(unsigned long x, const unsigned long *dp)
{
    char c;
    unsigned long d;
    if(x) {
        while(x < *dp) ++dp;
        do {
            d = *dp++;
            c = '0';
            while(x >= d) ++c, x -= d;
            putc(c);
        } while(!(d & 1));
    } else
        putc('0');
}

static void puth(unsigned n)
{
    static const char hex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    putc(hex[n & 15]);
}

void printf(char *format, ...)
{
    char c;
    int i;
    long n;

    va_list a;
    va_start(a, format);
    while(c = *format++) {
        if(c == '%') {
            switch(c = *format++) {
                case 's':                       // String
                    puts(va_arg(a, char*));
                    break;
                case 'c':                       // Char
                    putc(va_arg(a, char));
                    break;
                case 'i':                       // 16 bit Integer
                case 'u':                       // 16 bit Unsigned
                    i = va_arg(a, int);
                    if(c == 'i' && i < 0) i = -i, putc('-');
                    xtoa((unsigned)i, dv + 5);
                    break;
                case 'l':                       // 32 bit Long
                case 'n':                       // 32 bit uNsigned loNg
                    n = va_arg(a, long);
                    if(c == 'l' &&  n < 0) n = -n, putc('-');
                    xtoa((unsigned long)n, dv);
                    break;
                case 'x':                       // 16 bit heXadecimal
                    i = va_arg(a, int);
                    puth(i >> 12);
                    puth(i >> 8);
                    puth(i >> 4);
                    puth(i);
                    break;
                case 0: return;
                default: goto bad_fmt;
            }
        } else
bad_fmt:    putc(c);
    }
    va_end(a);
}

