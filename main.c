/*
 * main.c
 *
 * Maciej Kucia January 2013
 *
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
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"

#include "util/uartstdio.h"

#include "TRF797x.h"

#include "BP_RFID_TRF.h"
#include "BP_ISO15693.h"
#include "BP_NDEF.h"

/// SysTick ///
///
unsigned volatile int SysTickCounter = 0;
unsigned volatile int SysTickCounter2 = 0;
void SysTickISR(void)
{
	if (++SysTickCounter >= 1000)
	{
		SysTickCounter = 0;
		++SysTickCounter2;
		//TODO: this is wrong
		//UARTFlushTx(false);
	}

	// update BP LEDs if blinking
	//BP_RFID_HW_LEDS_UPDATE(SysTickCounter);

	//timeout handling
	//BP_RFID_Tick();
}

void dbg_time(void)
{
	printf("\n%03d,%03d:", SysTickCounter2, SysTickCounter);
}

/// User code entry point ///
///
int main(void)
{
	char adr, value;
	//int i = 0;

	// Initialize
	//
	ROM_SysCtlClockSet (SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
	// 1ms SysTick
	SysTickPeriodSet(SysCtlClockGet() / 1000);
	SysTickIntEnable();
	SysTickEnable();

	// Serial console
	Launchpad_UART_Init();

	// Booster pack
	BP_RFID_Init();
	IntMasterEnable();

	printf("[START]\n");

	for (;;)
	{
		printf("> ");
		switch(UARTgetc())
		{
			case 'c':
				adr = UARTgetc();
				printf("[direct command 0x%02x]\n", adr);
				BP_RFID_TRF_Write_Command(adr);
				break;

			case 'w':
				adr = UARTgetc();
				value = UARTgetc();
				BP_RFID_TRF_Write_Register(adr, value);
				printf("[write 0x%02x < 0x%02x]\n");
				break;

			case 'r':
				adr = UARTgetc();
				printf("[read 0x%02x > 0x%02x]\n", adr, BP_RFID_TRF_Read_Register(adr));
				break;

			case 'x':
				BP_ISO15693_Init();
				break;

			case '\n':
			case '\r':
				break;

			default:
				printf("[unknown command]\n");
		}
	}
}

