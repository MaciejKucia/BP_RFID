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

#include "util/uartstdio.h"

#include "TRF797x.h"

#include "BP_RFID.h"
#include "BP_ISO15693.h"
#include "BP_NFC.h"

#define HEARTBEAT_LED GPIO_PIN_3

/// SysTick ///
///
unsigned volatile long SysTickCounter = 0;
void SysTickISR(void)
{
	if (SysTickCounter++ > 10000)
		SysTickCounter = 0;
	// update BP LEDs if blinking
	BP_RFID_LEDS_UPDATE(SysTickCounter);
}

/// User code entry point ///
///
int main(void)
{
	int i = 0;

	// Initialize
	//
	// Clocks
	ROM_SysCtlClockSet (
			SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ
					| SYSCTL_OSC_MAIN);
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
		char addr, value;

		switch (UARTgetc())
		{
		case 'r':
			printf("[read parallel");
			addr = UARTgetc();
			printf(":0x%X,", addr);
			printf("0x%x]\n", BP_RFID_HW_READ_PARALLEL(addr));
			break;
		case 'w':
			printf("[write parallel reg");
			addr = UARTgetc();
			printf(":0x%X,", addr);
			value = UARTgetc();
			printf("0x%X]\n", value);
			BP_RFID_HW_WRITE_PARALLEL(addr, value, 0);
			break;

		case 'R':
			for (i = 0; i < 0x14; ++i)
			{
				printf("[Read parallel:%d,0x%x]\n", i,
						BP_RFID_HW_READ_PARALLEL(i));
			}

			break;

			// Test
		case 'T':

			BP_ISO15693_Init();
			BP_ISO15693_Inventory();
			BP_ISO15693_Read_Single_Block(0);
			BP_ISO15693_Get_System_Information(0);


			break;

		case 'N':
			BP_RFID_NFC_Init();
			BP_RFID_NFC_Collision_Avoidance();
			break;

		case '0':
			BP_RFID_LED1(LED_BLINK_FAST);
			break;
		case '1':
			BP_RFID_LED1(LED_BLINK_SLOW);
			break;
		case '2':
			BP_RFID_LED2(LED_BLINK_FAST);
			break;
		case '3':
			BP_RFID_LED2(LED_BLINK_SLOW);
			break;

		}

	}

}

