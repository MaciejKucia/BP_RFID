/*
 * BP_RFID_HW.c
 *
 *  Created on: Jan 30, 2013
 *      Author: x0184343
 *
 *
 *      This file consists code for low-level interaction with booster pack hardware.
 *
 *
 */

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ssi.h"

#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/fpu.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"

#include "util/uartstdio.h"

#include "BP_RFID_TRF.h"
#include "TRF797x.h"

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
	//BP_RFID_TRF_IRQ_Clear();
	ROM_GPIOPinIntEnable(GPIO_PORTE_BASE, IRQ_PIN);

#ifdef DBG_PRINT
	printf("[Ion]");
#endif

}

/// Disable interrupts from chip
///
void BP_RFID_HW_INT_DISABLE()
{
	ROM_GPIOPinIntDisable(GPIO_PORTE_BASE, IRQ_PIN);

#ifdef DBG_PRINT
	printf("[Ioff]");
#endif

}

#ifdef BP_RFID_PARALLEL

/// Initialize hardware for parallel interface
///
void BP_RFID_HW_PARALLEL_INIT()
{
	//Step 1 - IO config
	if (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	if (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)) SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	if (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC)) SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	if (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)) SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	if (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)) SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	if (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Only LEDs use port A
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, LED1_PIN | LED2_PIN);

	// Port B is used by ASK_OOK MOD T2
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, ASK_OOK_PIN | MOD_PIN | T2_PIN);

	// Port C is used by T3-T6
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, T3_PIN | T4_PIN | T5_PIN | T6_PIN);

	// Port D is used by T7 T8
	//
	//First open the lock and select the bits we want to modify in the GPIO commit register.
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR) = 0x80;
	//
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, T7_PIN | T8_PIN);

	// Port E is used by IRQ (interrupt) and Data clk
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, DATA_CLK_PIN);
	//
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, IRQ_PIN);
	GPIOIntTypeSet(GPIO_PORTE_BASE, IRQ_PIN, GPIO_RISING_EDGE);
	GPIOPinIntEnable(GPIO_PORTE_BASE, IRQ_PIN);

	// Port F is used by EN1 and T1
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, EN1_PIN | T1_PIN);

	BP_RFID_HW_DISABLE();

	printf("[RFID: parallel]\n");
}

/// Set data clock high
///
void BP_RFID_HW_DATA_CLK_HIGH(void)
{
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, DATA_CLK_PIN);
	// At least 50ns
	ROM_SysCtlDelay(50*SysCtlClockGet()/1000000);
}

/// Set data clock low
///
void BP_RFID_HW_DATA_CLK_LOW()
{
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, 0);
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	ROM_SysCtlDelay(50*SysCtlClockGet()/1000000);
}

/// Put bits on data bus
/// This is not the fastest code on the world but it works
//
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
	//TODO: IFDEF
	//printf("[<%02X]",adrcmd);
}

/// Set bus as output (MCU)
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

/// Set bus as input (MCU)
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

	//TODO: IFDEF
	//printf("[>%02X]",ret);
	return ret;
}

/// Send start condition on parallel line
void BP_RFID_HW_PARALLEL_START()
{
	//BP_RFID_HW_PARALLEL_PUT(0x00);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, T1_PIN, 0);
	//BP_RFID_HW_DATA_CLK_HIGH();
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, DATA_CLK_PIN);
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	//BP_RFID_HW_PARALLEL_PUT(0xFF);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, T1_PIN, T1_PIN); //IO_[7]
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	//BP_RFID_HW_DATA_CLK_LOW();
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, 0);
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	//BP_RFID_HW_PARALLEL_PUT(0x00);
}

/// Send stop condition
void BP_RFID_HW_PARALLEL_STOP()
{
	//BP_RFID_HW_PARALLEL_PUT(0xFF);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, T1_PIN, T1_PIN); //IO_[7]
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	//BP_RFID_HW_DATA_CLK_HIGH();
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, DATA_CLK_PIN);
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	//BP_RFID_HW_PARALLEL_PUT(0x00);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, T1_PIN, 0); //IO_[7]
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	//BP_RFID_HW_DATA_CLK_LOW();
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, 0);
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
}

/// Send stop while in multiple mode
void BP_RFID_HW_PARALLEL_STOP_MULTIPLE()
{
	//BP_RFID_HW_PARALLEL_PUT(0x80);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, T1_PIN, T1_PIN);
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
	//BP_RFID_HW_PARALLEL_PUT(0x00);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, T1_PIN, 0);
	//ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
}

/// Clock data
///
void BP_RFID_HW_PARALLEL_SIMPLE_WRITE(char data)
{
	BP_RFID_HW_PARALLEL_PUT(data);

	//Clock
//	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, DATA_CLK_PIN);
//	ROM_SysCtlDelay(BP_RFID_HW_PERIOD);
//	ROM_GPIOPinWrite(GPIO_PORTE_BASE, DATA_CLK_PIN, 0);
//	ROM_SysCtlDelay(BP_RFID_HW_PERIOD);

	BP_RFID_HW_DATA_CLK_HIGH(); BP_RFID_HW_DATA_CLK_LOW();
}

/// Write data to register
///
void BP_RFID_HW_PARALLEL_WRITE(char adr, char data, char command)
{
	// step1 address + non continous [0] + write bit [0] + address mode [0]
	char adrcmd = (adr & 0x1F);
	if (command) adrcmd |= TRF_IS_COMMAND;

	// DEBUG
	//printf ("[ %x < %x ]",adrcmd,data);

	// start condition
	BP_RFID_HW_PARALLEL_START();

	// put address on bus
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE(adrcmd);

	if (!command)
		BP_RFID_HW_PARALLEL_SIMPLE_WRITE(data);

	BP_RFID_HW_PARALLEL_STOP();
}

/// Write data to registers
///
void BP_RFID_HW_PARALLEL_WRITE_MULTIPLE(char* data, char size)
{
	char i;

	// debug
	//printf ("[WM(%x)>",size);

	// start condition
	BP_RFID_HW_PARALLEL_START();

	for (i = 0; i < size; ++i)
		BP_RFID_HW_PARALLEL_SIMPLE_WRITE(data[i]);

	BP_RFID_HW_PARALLEL_STOP_MULTIPLE();
}

/// Read data from register
///
char BP_RFID_HW_PARALLEL_READ(char adr)
{
	// step1 address + non continous [0] + read bit [1] + address mode [0]
	char adrcmd = (adr & 0x1F) | TRF_DO_READ;

	// start condition
	BP_RFID_HW_PARALLEL_START();

	// put address on bus
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE(adrcmd);

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
void BP_RFID_HW_PARALLEL_READ_MULTIPLE(char adr, char* buffer, char size)
{
	int i;
	// step1 address + continous [1] + read bit [1] + address mode [0]
	char adrcmd = (adr & 0x1F) | TRF_CONTINOUS_MODE | TRF_DO_READ;

	//printf("[R (%x) %x <!!]",size,adrcmd);

	// start condition
	BP_RFID_HW_PARALLEL_START();

	// put address on bus
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE(adrcmd);

	BP_RFID_HW_PARALEL_SET_INPUT();
	BP_RFID_HW_PARALLEL_PUT(0);

	for (i = 0; i < size; ++i)
	{
		BP_RFID_HW_DATA_CLK_HIGH();
		buffer[i] = BP_RFID_HW_PARALLEL_GET();
		BP_RFID_HW_DATA_CLK_LOW();
	}

	BP_RFID_HW_PARALEL_SET_OUTPUT();

	BP_RFID_HW_PARALLEL_STOP_MULTIPLE();
}

#endif

#ifdef BP_RFID_SERIAL

/// Initializes hardware for serial use
/// Additional hardware setup required
///
void BP_RFID_HW_SERIAL_INIT(void)
{

	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, T6_PIN);

	// Set IO pins for SPI SS operation
	GPIOPinWrite(GPIO_PORTC_BASE, T6_PIN, 0);
	GPIOPinWrite(GPIO_PORTD_BASE, T8_PIN | T7_PIN, T8_PIN | T7_PIN);

	// Start SPI peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);

	GPIOPinConfigure(GPIO_PD0_SSI1CLK);
	GPIOPinConfigure(GPIO_PD1_SSI1FSS);
	GPIOPinConfigure(GPIO_PD2_SSI1RX);
	GPIOPinConfigure(GPIO_PD3_SSI1TX);

	GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

	// 2 [MHz] clock
	SSIConfigSetExpClk(SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 2000000, 8);

	SSIEnable(SSI1_BASE);

	// Clean input
	while (SSIDataGetNonBlocking(SSI1_BASE, 0))
		;

	printf("[RFID: serial]\n");


}

#endif
