/*
 * BP_RFID.c
 *
 * Main routines for TRF chip
 *
 *  Created on: Jan 31, 2013
 *      Author: x0184343
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

#include "BP_RFID_TRF.h"

#include "util/uartstdio.h"

#include "TRF797x.h"

#include <stdarg.h>

// TODO:
// Reader can work in 3 modes
// InitReader_DIRECT_MODE0
// InitReader_DIRECT_MODE1
// InitReader_DIRECT_MODE2

/// Values present in TRF7970A after power on
/// Only for debug
//
char TRF_DEFAULT_REG_POWERON[] =
{ 0x01, 0x21, 0x00, 0x00, 0xC1, 0xC1, 0x00, 0x0E, 0x07, 0x91, 0x10, 0x87, 0x00,
		0x3E, 0x00, 0x40 };

/// last IRQ flags
volatile char TRF_IRQ_LAST_FLAGS;

/// IRQ happened flags
volatile char TRF_IRQ_SEMAPHORE;

/// IRQ callback pointer
///
void (*BP_RFID_IRQ_Callback)(char) = 0;

// TO DO read end write end callbacks

/// Function registers new callback
///
void BP_RFID_Set_IRQ_Callback(void (*function)(char))
{
	BP_RFID_IRQ_Callback = function;
}

/// Main buffer
///
#define BP_RFID_BUFFER_SIZE 256
char BP_RFID_BUFFER[BP_RFID_BUFFER_SIZE];

/// Clear the buffer (mainly debug)
///
void BP_RFID_BUFFER_CLEAR(void)
{
	int i;
	for (i = 0; i < BP_RFID_BUFFER_SIZE; ++i)
		BP_RFID_BUFFER[i] = 0;
}

#define BP_RFID_RX_BUFFER_SIZE 40
char BP_RFID_RX_BUFFER[BP_RFID_RX_BUFFER_SIZE];
volatile char BP_RFID_RX_BUFFER_COUNT = 0;






/// Writes one register
///
void BP_RFID_TRF_Write_Register(char adr, char data)
{
#ifdef BP_RFID_PARALLEL

	BP_RFID_HW_PARALLEL_START();
	BP_RFID_HW_SIMPLE_WRITE((adr & 0x1F));
	BP_RFID_HW_SIMPLE_WRITE(data);
	BP_RFID_HW_PARALLEL_STOP();

	//BP_RFID_HW_WRITE_PARALLEL(adr, data, 0);
#endif
}

/// Writes one command
///
void BP_RFID_TRF_Write_Command(char cmd)
{
#ifdef BP_RFID_PARALLEL

	BP_RFID_HW_PARALLEL_START();
	BP_RFID_HW_SIMPLE_WRITE((cmd & 0x1F) | IS_COMMAND);
	BP_RFID_HW_PARALLEL_STOP();

	//BP_RFID_HW_WRITE_PARALLEL(data, 0, 1);
#endif
}

/// Reads one register
///
char BP_RFID_TRF_Read_Register(char adr)
{
#ifdef BP_RFID_PARALLEL
	return BP_RFID_HW_READ_PARALLEL(adr);
#endif
}

/// Reads multiple registers
///
void BP_RFID_TRF_Read_Registers(char adr, char* buffer, char size)
{
#ifdef BP_RFID_PARALLEL
	BP_RFID_HW_READ_PARALLEL_MULTIPLE(adr, buffer, size);
#endif
}

///
///
///
void BP_RFID_TRF_Idle()
{
	BP_RFID_TRF_Write_Command(IDLE);
}

void BP_RFID_TRF_Software_Init()
{
	// Reset
	BP_RFID_HW_DISABLE();
	BP_RFID_HW_ENABLE();

	BP_RFID_BUFFER_CLEAR();

	BP_RFID_TRF_Write_Command(SOFT_INIT);

	BP_RFID_TRF_Idle();

	//BP_RFID_TRF_Modulator_Control(TRF_MOD_OOK100);

}

void BP_RFID_TRF_Adjust_Gain()
{
	BP_RFID_TRF_Write_Command(ADJUST_GAIN);
}

void BP_RFID_TRF_Check_RF()
{
	BP_RFID_TRF_Write_Command(CHECK_INTERNAL_RF);
}

void BP_RFID_TRF_Check_AUX_RF()
{
	BP_RFID_TRF_Write_Command(CHECK_EXTERNAL_RF);
}

void BP_RFID_TRF_IRQ_Clear()
{
	BP_RFID_TRF_Write_Register(TRF_REG_IRQ_STATUS, 0x00);
}

void BP_RFID_TRF_Initial_RF_Collision()
{
	BP_RFID_TRF_Write_Command(INITIAL_RF_COLLISION);
}

void BP_RFID_TRF_Response_RF_Collision_N()
{
	BP_RFID_TRF_Write_Command(RESPONSE_RF_COLLISION_N);
}
void BP_RFID_TRF_Initial_RF_Collision_0()
{
	BP_RFID_TRF_Write_Command(RESPONSE_RF_COLLISION_0);
}

void BP_RFID_TRF_Run_Decoders()
{
	BP_RFID_TRF_Write_Command(RUN_DECODERS);
}

void BP_RFID_TRF_Stop_Decoders()
{
	BP_RFID_TRF_Write_Command(STOP_DECODERS);
}

void BP_RFID_TRF_Reset_Decoders()
{
	BP_RFID_TRF_Stop_Decoders();
	BP_RFID_TRF_Run_Decoders();
}

void BP_RFID_TRF_Transmit_Next_Slot()
{
	BP_RFID_TRF_Write_Command(TRANSMIT_NEXT_SLOT);
}

void BP_RFID_TRF_Transmit_Close_Slot_Sequence()
{
	BP_RFID_TRF_Write_Command(CLOSE_SLOT_SEQUENCE);
}

/// Turns radio off
///
void BP_RFID_TRF_Turn_RF_Off()
{
	BP_RFID_HW_LED1(LED_OFF);
	// RF_ON bit off
	char reg = BP_RFID_TRF_Read_Register(TRF_REG_CHIP_STATE_CONTROL);
	BP_RFID_TRF_Write_Register(TRF_REG_CHIP_STATE_CONTROL, reg & ~(TRF_CSR_RF_ON));
}

/// Turns radio on
///
void BP_RFID_TRF_Turn_RF_On()
{
	BP_RFID_HW_LED1(LED_BLINK_SLOW);
	// RF_ON bit on
	char reg = BP_RFID_TRF_Read_Register(TRF_REG_CHIP_STATE_CONTROL);
	BP_RFID_TRF_Write_Register(TRF_REG_CHIP_STATE_CONTROL, reg | (TRF_CSR_RF_ON));
}

void BP_RFID_TRF_Modulator_Control(char control)
{
	BP_RFID_TRF_Write_Register(TRF_REG_MODULATOR_CONTROL, control);
}

char BP_RFID_TRF_Get_ISO(void)
{
	return BP_RFID_TRF_Read_Register(TRF_REG_ISO_CONTROL);
}

/// Returns 0 when non-rfid mode
///
char BP_RFID_TRF_Is_NFC(void)
{
	return BP_RFID_TRF_Read_Register(TRF_REG_ISO_CONTROL) & TRF_PROTOCOL_RFID;
}

void BP_RFID_TRF_Set_ISO(char iso)
{
	BP_RFID_TRF_Write_Register(TRF_REG_ISO_CONTROL, iso);
}

char BP_RFID_TRF_Is_OSC_Stable(void)
{
	return (BP_RFID_TRF_Read_Register(TRF_REG_RSSI_LEVELS) & (1 << 6) ? 1 : 0);
}

/// Returns RSSI value 0-7 where 7 is max and 0 is min
///
char BP_RFID_TRF_Get_RSSI(void)
{
	return (BP_RFID_TRF_Read_Register(TRF_REG_RSSI_LEVELS) & 0x07);
}

/// TODO: What is AUX??
///
char BP_RFID_TRF_Get_AUX_RSSI(void)
{
	return ((BP_RFID_TRF_Read_Register(TRF_REG_RSSI_LEVELS) & 0x38) >> 3);
}

/// Resets FIFO
///
void BP_RFID_TRF_FIFO_Reset(void)
{
	BP_RFID_TRF_Write_Command(RESET);
}

/// Returns data count in FIFO
//
char BP_RFID_TRF_FIFO_How_Many_Bytes(void)
{
	return BP_RFID_TRF_Read_Register(TRF_REG_FIFO_CONTROL) & 0x7F;
}

/// Is FIFO overflowed?
///
char BP_RFID_TRF_FIFO_Is_OVF(void)
{
	return (BP_RFID_TRF_Read_Register(TRF_REG_FIFO_CONTROL) & (1 << 7) ? 1 : 0);
}

//TODO
void BP_RFID_TRF_Transmit_Delay_No_CRC()
{
	BP_RFID_TRF_Write_Command(DELAY_TRANSMIT_NO_CRC);
}

//TODO
void BP_RFID_TRF_Transmit_Delay()
{
	BP_RFID_TRF_Write_Command(DELAY_TRANSMIT_CRC);
}

//TODO
void BP_RFID_TRF_Transmit_No_CRC()
{
	BP_RFID_TRF_Write_Command(DELAY_TRANSMIT_NO_CRC);
}

/// Function Sets unique NFC ID number
///
/// TODO: array pointer better?
void BP_RFID_TRF_Set_NFCID1(char size, ...)
{
	va_list vaArgP;
	char i = 0;

	if ((size != 4) && (size != 7) && (size != 10))
	{
		printf(
				"BP_RFID_TRF_Set_NFCID1 Error: wrong argument! use size 4, 7 or 10");
		return;
	}

	va_start(vaArgP, size);

	BP_RFID_BUFFER[i] = TRF_REG_NFCID1_Number | CONTINOUS_MODE;
	while (size--)
		BP_RFID_BUFFER[++i] = va_arg(vaArgP, char);
	BP_RFID_HW_WRITE_PARALLEL_MULTIPLE(BP_RFID_BUFFER, i);

	va_end(vaArgP);
}

/// TODO: this do not fit here
///
void BP_RFID_TRF_NFC_Target_Protocol_DEBUG()
{
	char r = BP_RFID_TRF_Read_Register(TRF_REG_NFC_Target_Protocol);
	printf("[Target_Protocol:");
	if (r&TRF_NFC_Target_Protocol_RF_Level_wake)
		printf("(RF level is above the wake-up level setting)" );

	if (r&TRF_NFC_Target_Protocol_RF_Level_collision)
		printf("(RF level is above the RF collision avoidance level setting)" );

	printf("(The first initiator command had physical level coding of %s)",r&TRF_NFC_Target_Protocol_FeliCa?"FeliCa":"ISO14443A");

	if (r&TRF_NFC_Target_Protocol_PassiveOrTag)
		printf("(Passive target at 106 kbps or transponder emulation)" );

	if (r&TRF_NFC_Target_Protocol_ISO14443B)
		printf("(The first reader command was ISO14443B)" );

	switch(r&0x3)
	{
		case 1:	printf("(106kbps)"); break;
		case 2:	printf("(212kbps)"); break;
		case 3:	printf("(424kbps)"); break;
		default: printf("(ERROR)");  break;
	}
	 printf("]\n");
}

/// Transmits packet of data trough radio
///
void BP_RFID_TRF_Transmit(char *data, unsigned short size)
{
	short i;

	printf("[Tx");

	// Wait with interrupts until we finish
	BP_RFID_HW_INT_DISABLE();

	TRF_IRQ_LAST_FLAGS = BP_RFID_TRF_Read_Register(TRF_REG_IRQ_STATUS);

	// TODO:
	if (size > 64)
		return;

	BP_RFID_HW_PARALLEL_START();

	BP_RFID_HW_SIMPLE_WRITE(RESET | IS_COMMAND);
	BP_RFID_HW_SIMPLE_WRITE(TRF_REG_TX_LENGTH_BYTE_1 | CONTINOUS_MODE); // write byte count from 0x1D
	// see Table 6-35. TX Length Byte2 Register (0x1E) 4 LSB bytes are for broken byte
	//size = size << 4;
	BP_RFID_HW_SIMPLE_WRITE((size & 0x0FF0) >> 4);
	BP_RFID_HW_SIMPLE_WRITE((size & 0x000F) << 4);

	BP_RFID_HW_SIMPLE_WRITE(TRANSMIT_CRC | IS_COMMAND);

	// this will be written to FIFO register 0x1F (increment mode)
	for (i = 0; i < size; ++i)
		BP_RFID_HW_SIMPLE_WRITE(data[i]);

	BP_RFID_HW_PARALLEL_STOP_MULTIPLE();
	BP_RFID_TRF_Read_Register(TRF_REG_IRQ_STATUS);

	BP_RFID_HW_INT_ENABLE();

	//Wait for Tx end
	BP_RFID_TRF_Wait_For_Tx_End();

	printf("]");

	//BP_RFID_HW_WRITE_PARALLEL_MULTIPLE(BP_RFID_BUFFER, size + 4);

}

void BP_RFID_Init()
{
	int i;

	TRF_IRQ_LAST_FLAGS = TRF_IRQ_SEMAPHORE = 0;

#ifdef BP_RFID_PARALLEL

	//Step 1 - IO config

	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	// ^ Configured in UART TODO:check

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Only LEDs use port A
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, LED1_PIN | LED2_PIN);

	// Port B is used by ASK_OOK MOD T2
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, ASK_OOK_PIN | MOD_PIN | T2_PIN);

	// Port C is used by T3-T6
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, T3_PIN | T4_PIN | T5_PIN | T6_PIN);

	// Port D is used by T7 T9
	//
	//First open the lock and select the bits we want to modify in the GPIO commit register.
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR) = 0x80;
	//
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, T7_PIN | T8_PIN);

	// Port E is used by IRQ (interrupt) and Data clk
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, DATA_CLK_PIN);

	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, IRQ_PIN);
	GPIOIntTypeSet(GPIO_PORTE_BASE, IRQ_PIN, GPIO_RISING_EDGE);
	GPIOPinIntEnable(GPIO_PORTE_BASE, IRQ_PIN);
	IntEnable(INT_GPIOE);

	// Port F is used by EN1 and T1
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, EN1_PIN | T1_PIN);

	printf("[RFID: parallel]\n");

	// Set LEDs
	BP_RFID_HW_LED1(LED_ON);
	BP_RFID_HW_LED2(LED_ON);

	BP_RFID_TRF_Software_Init();

	// Check if registers are OK
	BP_RFID_HW_READ_PARALLEL_MULTIPLE(0, BP_RFID_BUFFER, 16);
	for (i = 0; i < 16; ++i)
		if (TRF_DEFAULT_REG_POWERON[i] != BP_RFID_BUFFER[i])
			printf(
					"[W! Def Reg mismatch @0x%X (%x != %x)]\n",
					i, TRF_DEFAULT_REG_POWERON[i], BP_RFID_BUFFER[i]);

	// Set LEDs
	BP_RFID_HW_LED1(LED_OFF);
	BP_RFID_HW_LED2(LED_OFF);
#endif
}

/// IRQ input handler
///
void IRQ_ISR(void)
{
	char c;

	// read int register and clear it
	TRF_IRQ_LAST_FLAGS = BP_RFID_TRF_Read_Register(TRF_REG_IRQ_STATUS);

	TRF_IRQ_SEMAPHORE = 1;

	printf("[I(%d)",TRF_IRQ_LAST_FLAGS);

//	// Tx is finished now
	if (TRF_IRQ_LAST_FLAGS&TRF_IRQ_TX)
	{
		printf("T");

		BP_RFID_TRF_Stop_Decoders();
		BP_RFID_TRF_Run_Decoders();
		BP_RFID_TRF_FIFO_Reset();
		BP_RFID_TRF_Read_Register(TRF_REG_IRQ_STATUS); //?
	}

	// Rx is finished
	if (TRF_IRQ_LAST_FLAGS & TRF_IRQ_RX)
	{
		printf("R");
		BP_RFID_RX_BUFFER_COUNT = 0;

		while (c = BP_RFID_TRF_FIFO_How_Many_Bytes())
		{
			printf("(%d)", c);
			BP_RFID_TRF_Read_Registers(TRF_REG_FIFO, BP_RFID_RX_BUFFER, c);
			BP_RFID_RX_BUFFER_COUNT += c;
		}

		BP_RFID_TRF_Stop_Decoders();
		BP_RFID_TRF_Run_Decoders();
		BP_RFID_TRF_FIFO_Reset();
		BP_RFID_TRF_Read_Register(TRF_REG_IRQ_STATUS); //?

		//for (i = 0; i < BP_RFID_RX_BUFFER_COUNT; ++i)
		//	printf("(%x)", BP_RFID_RX_BUFFER[i]);
	}

	// FIFO is low or high
	if (TRF_IRQ_LAST_FLAGS&TRF_IRQ_FIFO)
	{

		//TODO: inplement
	}

	ROM_GPIOPinIntClear(GPIO_PORTE_BASE, IRQ_PIN);

	if(BP_RFID_IRQ_Callback!=0)
		BP_RFID_IRQ_Callback(TRF_IRQ_LAST_FLAGS);

	BP_RFID_TRF_Write_Register(TRF_REG_IRQ_STATUS, 0x00);

	printf("]\n");


}

/// TODO: implement timeout
char BP_RFID_TRF_Wait_For_Rx_End(void)
{
	TRF_IRQ_SEMAPHORE=0;
	while(TRF_IRQ_SEMAPHORE==0);
	if (TRF_IRQ_LAST_FLAGS & TRF_IRQ_RX) return 0;
	return 1;
}

char BP_RFID_TRF_Wait_For_Tx_End(void)
{
	TRF_IRQ_SEMAPHORE=0;
	while(TRF_IRQ_SEMAPHORE==0);
	if (TRF_IRQ_LAST_FLAGS & TRF_IRQ_TX) return 0;
	return 1;
}


