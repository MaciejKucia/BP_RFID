/*
 * BP_RFID_HW.c
 *
 *  Created on: Jan 30, 2013
 *      Author: x0184343
 *
 *
 *      This file consists code for low-level interaction with hardware.
 *
 *
 */

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"

#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "util/uartstdio.h"

#include "BP_RFID_TRF.h"

// Recommended DATA_CLK is 2 [MHz]
// 2 MHz = period of 0.5 [us] approx 1 [us]
// This is experimental value
#define BP_RFID_HW_PERIOD 1

/// Booster Pack LED ///
///

/// LED flags for blinking
unsigned char LEDS_FLAGS = 0;

// mode:
// 0 - off
// 1 - on
// 2 - toggle
// 3 - blink fast
// 4 - blink slow

/// Set mode for LED1
///
void BP_RFID_HW_LED1(char mode)
{
	switch (mode)
	{
	case LED_OFF:
		LEDS_FLAGS = 0;
		ROM_GPIOPinWrite(GPIO_PORTA_BASE, LED1_PIN, LED1_PIN);
		break;
	case LED_ON:
		LEDS_FLAGS = 0;
		ROM_GPIOPinWrite(GPIO_PORTA_BASE, LED1_PIN, 0);
		break;
	case LED_TOGGLE:
		LEDS_FLAGS = 0;
		ROM_GPIOPinWrite(
				GPIO_PORTA_BASE,
				LED1_PIN,
				~ROM_GPIOPinRead(GPIO_PORTA_BASE, LED1_PIN));
		break;
	case LED_BLINK_FAST:
		LEDS_FLAGS = ((LEDS_FLAGS & 0xF0) | 0x0A);
		break;
	case LED_BLINK_SLOW:
		LEDS_FLAGS = ((LEDS_FLAGS & 0xF0) | 0x0B);
		break;
	}
}

/// Set mode for LED2
///
void BP_RFID_HW_LED2(char mode)
{
	switch (mode)
	{
	case LED_OFF:
		LEDS_FLAGS = 0;
		ROM_GPIOPinWrite(GPIO_PORTA_BASE, LED2_PIN, LED2_PIN);
		break;
	case LED_ON:
		LEDS_FLAGS = 0;
		ROM_GPIOPinWrite(GPIO_PORTA_BASE, LED2_PIN, 0);
		break;
	case LED_TOGGLE:
		LEDS_FLAGS = 0;
		ROM_GPIOPinWrite(
				GPIO_PORTA_BASE,
				LED2_PIN,
				~ROM_GPIOPinRead(GPIO_PORTA_BASE, LED2_PIN));
		break;
	case LED_BLINK_FAST:
		LEDS_FLAGS = ((LEDS_FLAGS & 0x0F) | 0xA0);
		break;
	case LED_BLINK_SLOW:
		LEDS_FLAGS = ((LEDS_FLAGS & 0x0F) | 0xB0);
		break;
	}
}

/// Set leds during sys tick update
///
void BP_RFID_HW_LEDS_UPDATE(unsigned long SysTickCounter)
{
	if ((LEDS_FLAGS & 0x0F) == 0x0A)
	{
		ROM_GPIOPinWrite(
				GPIO_PORTA_BASE,
				LED1_PIN,
				((SysTickCounter % 100) > 25) ? LED1_PIN : 0);
	}
	else if ((LEDS_FLAGS & 0x0F) == 0x0B)
	{
		ROM_GPIOPinWrite(
				GPIO_PORTA_BASE,
				LED1_PIN,
				((SysTickCounter % 500) > 250) ? LED1_PIN : 0);
	}

	if ((LEDS_FLAGS & 0xF0) == 0xA0)
	{
		ROM_GPIOPinWrite(
				GPIO_PORTA_BASE,
				LED2_PIN,
				((SysTickCounter % 100) > 25) ? LED2_PIN : 0);
	}
	else if ((LEDS_FLAGS & 0xF0) == 0xB0)
	{
		ROM_GPIOPinWrite(
				GPIO_PORTA_BASE,
				LED2_PIN,
				((SysTickCounter % 500) > 250) ? LED2_PIN : 0);
	}
}

/// Set RF chip active
///
void BP_RFID_HW_ENABLE()
{
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, EN1_PIN, EN1_PIN);
}

/// Disable RF chip
///
void BP_RFID_HW_DISABLE()
{
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, EN1_PIN, 0);
}

/// Enable interrupts from chip
///
void BP_RFID_HW_INT_ENABLE()
{
	ROM_GPIOPinIntEnable(GPIO_PORTE_BASE, IRQ_PIN);
}

/// Disable interrupts from chip
///
void BP_RFID_HW_INT_DISABLE()
{
	ROM_GPIOPinIntDisable(GPIO_PORTE_BASE, IRQ_PIN);
}


/// Set clock high
///
inline void BP_RFID_HW_DATA_CLK_HIGH(void)
{
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, DATA_CLK_PIN);
	ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
}

/// Set clock low
///
inline void BP_RFID_HW_DATA_CLK_LOW()
{
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, 0);
	ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
}

/// Put bits on data bus
///
void BP_RFID_HW_PARALLEL_PUT(char adrcmd)
{
	ROM_GPIOPinWrite(GPIO_PORTD_BASE, T8_PIN, adrcmd & (1 << 0) ? T8_PIN : 0);
	ROM_GPIOPinWrite(GPIO_PORTD_BASE, T7_PIN, adrcmd & (1 << 1) ? T7_PIN : 0);

	ROM_GPIOPinWrite(GPIO_PORTC_BASE, T6_PIN, adrcmd & (1 << 2) ? T6_PIN : 0);
	ROM_GPIOPinWrite(GPIO_PORTC_BASE, T5_PIN, adrcmd & (1 << 3) ? T5_PIN : 0);
	ROM_GPIOPinWrite(GPIO_PORTC_BASE, T4_PIN, adrcmd & (1 << 4) ? T4_PIN : 0);
	ROM_GPIOPinWrite(GPIO_PORTC_BASE, T3_PIN, adrcmd & (1 << 5) ? T3_PIN : 0);

	ROM_GPIOPinWrite(GPIO_PORTB_BASE, T2_PIN, adrcmd & (1 << 6) ? T2_PIN : 0);

	ROM_GPIOPinWrite(GPIO_PORTF_BASE, T1_PIN, adrcmd & (1 << 7) ? T1_PIN : 0);
}

/// Set bus as output from MCU
///
void BP_RFID_HW_PARALEL_SET_OUTPUT()
{	// Port B is used by T2
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, T2_PIN);

	// Port C is used by T3-T6
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, T3_PIN | T4_PIN | T5_PIN | T6_PIN);

	// Port D is used by T7 T9
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, T7_PIN | T8_PIN);

	// Port F is used by  T1
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, T1_PIN);
}

/// Set bus as input from mcu
///
void BP_RFID_HW_PARALEL_SET_INPUT()
{
	// Port B is used by T2
	ROM_GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, T2_PIN);

	// Port C is used by T3-T6
	ROM_GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, T3_PIN | T4_PIN | T5_PIN | T6_PIN);

	// Port D is used by T7 T9
	ROM_GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, T7_PIN | T8_PIN);

	// Port F is used by  T1
	ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, T1_PIN);
}

/// Read data from bus
///
char BP_RFID_HW_PARALLEL_GET()
{
	char ret = 0;
	ret |= (ROM_GPIOPinRead(GPIO_PORTF_BASE, T1_PIN) ? 1 << 7 : 0);

	ret |= (ROM_GPIOPinRead(GPIO_PORTB_BASE, T2_PIN) ? 1 << 6 : 0);

	ret |= (ROM_GPIOPinRead(GPIO_PORTC_BASE, T3_PIN) ? 1 << 5 : 0);
	ret |= (ROM_GPIOPinRead(GPIO_PORTC_BASE, T4_PIN) ? 1 << 4 : 0);
	ret |= (ROM_GPIOPinRead(GPIO_PORTC_BASE, T5_PIN) ? 1 << 3 : 0);
	ret |= (ROM_GPIOPinRead(GPIO_PORTC_BASE, T6_PIN) ? 1 << 2 : 0);

	ret |= (ROM_GPIOPinRead(GPIO_PORTD_BASE, T7_PIN) ? 1 << 1 : 0);
	ret |= (ROM_GPIOPinRead(GPIO_PORTD_BASE, T8_PIN) ? 1 << 0 : 0);

	return ret;
}

/// Send start condition on parallel line
void BP_RFID_HW_PARALLEL_START()
{
	BP_RFID_HW_PARALLEL_PUT(0x00);
	BP_RFID_HW_DATA_CLK_HIGH();
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	BP_RFID_HW_PARALLEL_PUT(0xFF);
	ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	BP_RFID_HW_DATA_CLK_LOW();
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	BP_RFID_HW_PARALLEL_PUT(0x00);
}

/// Send stop condition
void BP_RFID_HW_PARALLEL_STOP()
{
	BP_RFID_HW_PARALLEL_PUT(0xFF);
	ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	BP_RFID_HW_DATA_CLK_HIGH();
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	BP_RFID_HW_PARALLEL_PUT(0x00);
	ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	BP_RFID_HW_DATA_CLK_LOW();
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
}

/// Send stop while in multiple mode
void BP_RFID_HW_PARALLEL_STOP_MULTIPLE()
{
	BP_RFID_HW_PARALLEL_PUT(0x80);
	ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	BP_RFID_HW_PARALLEL_PUT(0x00);
	ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
}

/// Clock data
///
void BP_RFID_HW_SIMPLE_WRITE(char data)
{
	BP_RFID_HW_PARALLEL_PUT(data);
	BP_RFID_HW_DATA_CLK_HIGH();
	BP_RFID_HW_DATA_CLK_LOW();
}

/// Write data to register
///
void BP_RFID_HW_WRITE_PARALLEL(char adr, char data, char command)
{
	// step1 address + non continous [0] + write bit [0] + address mode [0]
	char adrcmd = (adr & 0x1F);
	if (command) adrcmd |= IS_COMMAND;

	// DEBUG
	//printf ("[ %x < %x ]",adrcmd,data);

	// start condition
	BP_RFID_HW_PARALLEL_START();

	// put address on bus
	BP_RFID_HW_SIMPLE_WRITE(adrcmd);

	if (!command)
	{
		BP_RFID_HW_SIMPLE_WRITE(data);
	}

	BP_RFID_HW_PARALLEL_STOP();
}

/// Write data to registers
///
void BP_RFID_HW_WRITE_PARALLEL_MULTIPLE(char* data, char size)
{
	int i;

	// debug
	//printf ("[WM(%x)>",size);

	// start condition
	BP_RFID_HW_PARALLEL_START();

	for (i = 0; i < size; ++i)
	{
		// put data
		BP_RFID_HW_SIMPLE_WRITE(data[i]);
	}

	//printf("]");

	BP_RFID_HW_PARALLEL_STOP_MULTIPLE();
}

/// Read data from register
///
char BP_RFID_HW_READ_PARALLEL(char adr)
{
	// step1 address + non continous [0] + read bit [1] + address mode [0]
	char adrcmd = (adr & 0x1F) | DO_READ;

	// start condition
	BP_RFID_HW_PARALLEL_START();

	// put address on bus
	BP_RFID_HW_SIMPLE_WRITE(adrcmd);

	BP_RFID_HW_PARALEL_SET_INPUT();
	BP_RFID_HW_PARALLEL_PUT(0);

	BP_RFID_HW_DATA_CLK_HIGH();

	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	//printf ("[r %x > %x]",(adr & 0x1F) | (1 << 6),
	adrcmd = BP_RFID_HW_PARALLEL_GET();
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	BP_RFID_HW_DATA_CLK_LOW();

	BP_RFID_HW_PARALEL_SET_OUTPUT();

	BP_RFID_HW_PARALLEL_STOP();

	return adrcmd;
}

/// Read data from register
///
void BP_RFID_HW_READ_PARALLEL_MULTIPLE(char adr, char* buffer, char size)
{
	int i;
	// step1 address + continous [1] + read bit [1] + address mode [0]
	char adrcmd = (adr & 0x1F) | CONTINOUS_MODE | DO_READ;

	//printf("[R (%x) %x <!!]",size,adrcmd);

	// start condition
	BP_RFID_HW_PARALLEL_START();

	// put address on bus
	BP_RFID_HW_SIMPLE_WRITE(adrcmd);

	BP_RFID_HW_PARALEL_SET_INPUT();
	BP_RFID_HW_PARALLEL_PUT(0);

	for (i = 0; i < size; ++i)
	{
		BP_RFID_HW_DATA_CLK_HIGH();
		//SysCtlDelay(BP_RFID_HW_PERIOD);
		buffer[i] = BP_RFID_HW_PARALLEL_GET();
		//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
		BP_RFID_HW_DATA_CLK_LOW();
	}

	BP_RFID_HW_PARALEL_SET_OUTPUT();

	BP_RFID_HW_PARALLEL_STOP_MULTIPLE();
}
