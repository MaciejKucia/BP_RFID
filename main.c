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
	int j = 0;
	char s,p;

	// Initialize
	//
	// Clocks
	ROM_SysCtlClockSet(
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

			for (j=0;j<1000;++j)
			{
				SysCtlDelay(2000000);

			BP_RFID_TRF_Software_Init();
			BP_RFID_TRF_Modulator_Control(MOD_OOK100);
			BP_RFID_TRF_Set_ISO(ISO_PROTOCOL_2);
			BP_RFID_TRF_Turn_RF_On();

			// magic
			BP_RFID_BUFFER[0] = 0x8F;
			BP_RFID_BUFFER[1] = 0x91;
			BP_RFID_BUFFER[2] = 0x3D;
			BP_RFID_BUFFER[3] = 0x00;
			BP_RFID_BUFFER[4] = 0x30;
			BP_RFID_BUFFER[5] = 0x26;
			BP_RFID_BUFFER[6] = 0x01;
			BP_RFID_BUFFER[7] = 0x00;
			BP_RFID_HW_WRITE_PARALLEL_MULTIPLE(BP_RFID_BUFFER,8);

			// WAIT FOR INT
			SysCtlDelay(2*SysCtlClockGet() / 1000);
			printf("\n=====\n");

			//BP_RFID_HW_READ_PARALLEL_MULTIPLE(IRQ_STATUS,BP_RFID_BUFFER,2);
			//printf("IRQ_STATUS:0x%x 0x%x\n", BP_RFID_BUFFER[0],BP_RFID_BUFFER[1]);

			//BP_RFID_Read_Register(FIFO_CONTROL);

			s = BP_RFID_TRF_FIFO_How_Many_Bytes();

			printf("%d bytes in FIFO\n",s);

			BP_RFID_Read_Registers(0x1F, BP_RFID_BUFFER, s);

			printf("%d bytes in FIFO\n",BP_RFID_Read_Register(FIFO_CONTROL));

			printf("[RSSI:%d%%]\n", p = BP_RFID_TRF_Get_RSSI());
			BP_RFID_LED2(p);

			//BP_RFID_Write_Register(0x0F, 0x00);
			//BP_RFID_Read_Register(0x0F);

			BP_RFID_TRF_Turn_RF_Off();

			for (i = 0; i < s; ++i)
				printf("(%x)", BP_RFID_BUFFER[i]);

			BP_RFID_TRF_FIFO_Reset();

			}
			break;

		case '0': BP_RFID_LED1(LED_BLINK_FAST); break;
		case '1': BP_RFID_LED1(LED_BLINK_SLOW); break;
		case '2': BP_RFID_LED2(LED_BLINK_FAST); break;
		case '3': BP_RFID_LED2(LED_BLINK_SLOW); break;

		}

	}

}
