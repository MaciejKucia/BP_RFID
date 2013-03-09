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
#include "driverlib/timer.h"
#include "driverlib/ssi.h"

#include "util/uartstdio.h"

#include "TRF797x.h"
#include "BP_RFID_TRF.h"

//#include <stdarg.h>

// --- Variables ------------------------------------------------------------------------

/// Values present in TRF7970A after power on
/// Only for debug
//
#define TRF_DEFAULT_REG_POWERON_COUNT 16
char TRF_DEFAULT_REG_POWERON[TRF_DEFAULT_REG_POWERON_COUNT] =
{ 0x01, 0x21, 0x00, 0x00, 0xC1, 0xC1, 0x00, 0x0E, 0x07, 0x91, 0x10, 0x87, 0x00, 0x3E, 0x00, 0x40 };

/// Buffer for NFC ID storage
///
char TRF_NFCID[10];

/// last IRQ flags
///
volatile char TRF_IRQ_LAST_FLAGS;

/// IRQ semaphore
///
volatile char TRF_IRQ_SEMAPHORE;

/// IRQ callback pointer
///
void (*BP_RFID_IRQ_Callback)(char) = 0;

/// Rx Tx timeout counter

// timeout
#define BP_RFID_TIMEOUT 300
unsigned short timeout;

// --- Buffers ----------------------------------------------------------------------------

#define 		BP_RFID_RX_BUFFER_SIZE 40
char BP_RFID_RX_BUFFER[BP_RFID_RX_BUFFER_SIZE];
volatile char BP_RFID_RX_BUFFER_COUNT = 0;

// --- Functions -------------------------------------------------------------------------

/// Writes one register
///
void BP_RFID_TRF_Write_Register(char adr, char data)
{
#ifdef BP_RFID_PARALLEL
	BP_RFID_HW_PARALLEL_START();
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE((adr & 0x1F));
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE(data);
	BP_RFID_HW_PARALLEL_STOP();
#else

	SSIDataPut(SSI1_BASE, (adr & 0x1F));
	SSIDataPut(SSI1_BASE, data);
	while(SSIBusy(SSI1_BASE));
#endif
}

/// Writes one command
///
void BP_RFID_TRF_Write_Command(char cmd)
{
#ifdef BP_RFID_PARALLEL
	BP_RFID_HW_PARALLEL_START();
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE((cmd & 0x1F) | TRF_IS_COMMAND);
	BP_RFID_HW_PARALLEL_STOP();
#else

	SSIDataPut(SSI1_BASE, (cmd & 0x1F) | TRF_IS_COMMAND);
//TODO: SPECIAL CONDITION !!!
	while(SSIBusy(SSI1_BASE));
#endif
}

/// Reads one register
///
char BP_RFID_TRF_Read_Register(char adr)
{
#ifdef BP_RFID_PARALLEL
	return BP_RFID_HW_PARALLEL_READ(adr);
#else
	unsigned long data[1];
	SSIDataPut(SSI1_BASE, (adr & 0x1F));
	while(SSIBusy(SSI1_BASE));
	SSIDataGet(SSI1_BASE, data);
	while(SSIBusy(SSI1_BASE));
	return (char)data[0];
	// else
	//	 return 0; //ERROR!
#endif
}

/// Reads multiple registers
///
void BP_RFID_TRF_Read_Registers(char adr, char* buffer, char size)
{
#ifdef BP_RFID_PARALLEL
	BP_RFID_HW_PARALLEL_READ_MULTIPLE(adr, buffer, size);
#else

#endif
}

/// TRF enters idle state
///
void BP_RFID_TRF_Idle()
{
#ifdef dbg_irq
	printf("[Idle_80]");
#endif
	BP_RFID_TRF_Write_Command(TRF_CMD_IDLE);
}

/// Starts frame decoders
///
void BP_RFID_TRF_Run_Decoders()
{
#ifdef dbg_irq
	printf("[RD]");
#endif
	BP_RFID_TRF_Write_Command(TRF_CMD_RUN_DECODERS);
}

/// Stops frame decoders
///
void BP_RFID_TRF_Stop_Decoders()
{
#ifdef dbg_irq
	printf("[SD]");
#endif
	BP_RFID_TRF_Write_Command(TRF_CMD_STOP_DECODERS);
}

/// Resets frame decoders
///
void BP_RFID_TRF_Reset_Decoders()
{
#ifdef dbg_irq
	printf("[RD]");
#endif
	BP_RFID_TRF_Write_Command(TRF_CMD_STOP_DECODERS);
	BP_RFID_TRF_Write_Command(TRF_CMD_RUN_DECODERS);
}

/// Turns radio off
///
void BP_RFID_TRF_Turn_RF_Off()
{
#ifdef dbg_irq
	printf("[RF-]");
#endif
	BP_RFID_HW_LED1(LED_OFF);
	// RF_ON bit off
	char reg = BP_RFID_TRF_Read_Register(TRF_REG_CHIP_STATE_CONTROL);
	BP_RFID_TRF_Write_Register(TRF_REG_CHIP_STATE_CONTROL, reg & ~(TRF_CSR_RF_ON));
}

/// Turns radio on
///
void BP_RFID_TRF_Turn_RF_On()
{
#ifdef dbg_irq
	printf("[RF+]");
#endif
	BP_RFID_HW_LED1(LED_BLINK_SLOW);
	// RF_ON bit on
	BP_RFID_TRF_Write_Register(TRF_REG_CHIP_STATE_CONTROL, BP_RFID_TRF_Read_Register(TRF_REG_CHIP_STATE_CONTROL) | (TRF_CSR_RF_ON));
}

/// Sets modulation
///
void BP_RFID_TRF_Modulator_Control(char control)
{
#ifdef dbg_irq
	printf("[MC]");
#endif
	BP_RFID_TRF_Write_Register(TRF_REG_MODULATOR_CONTROL, control);
}

/// Sets current ISO mode
///
void BP_RFID_TRF_Set_ISO(char iso)
{
	BP_RFID_TRF_Write_Register(TRF_REG_ISO_CONTROL, iso);
}

/// Returns current ISO mode
///
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

/// Returns 1 when internal oscillator is stable
///
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
#ifdef dbg_irq
	printf("[FIFO reset]");
#endif
	BP_RFID_TRF_Write_Command(TRF_CMD_FIFO_RESET);
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

/// Function Sets unique NFC ID number
///
void BP_RFID_TRF_Set_NFCID(char size, char* bytes)
{
	char i = 0;

	if ((size != 7) && (size != 10) && (size != 5))
	{
		printf("BP_RFID_TRF_Set_NFCID1 Error: wrong argument! use size 4, 7 or 10");
		return;
	}

#ifdef BP_RFID_PARALLEL

	BP_RFID_HW_PARALLEL_START();

	BP_RFID_HW_PARALLEL_SIMPLE_WRITE(TRF_REG_NFCID1_Number | TRF_CONTINOUS_MODE);

	for (i = 0; i < size; ++i)
	{
		TRF_NFCID[i] = bytes[i];
		BP_RFID_HW_PARALLEL_SIMPLE_WRITE(bytes[i]);
	}
	BP_RFID_HW_PARALLEL_STOP_MULTIPLE();

#else

#endif
}

/// Transmits packet of data trough radio
///
/// TODO: transmit no CRC, transmit delay
/// Returns eiter TRF_ERROR or TRF_NO_ERROR
char BP_RFID_TRF_Transmit(char *data, unsigned short size)
{
	short i;

	printf("[Tx... ");

	// TODO: handle big packet size
	if (size > 64) return 1;

	// Wait with interrupts until we finish
	BP_RFID_HW_INT_DISABLE();

#ifdef BP_RFID_PARALLEL

	BP_RFID_HW_PARALLEL_START();

	BP_RFID_HW_PARALLEL_SIMPLE_WRITE(TRF_CMD_FIFO_RESET | TRF_IS_COMMAND);
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE(TRF_CMD_TRANSMIT_CRC | TRF_IS_COMMAND);
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE(TRF_REG_TX_LENGTH_BYTE_1 | TRF_CONTINOUS_MODE); // write byte count from 0x1D

	// see Table 6-35. TX Length Byte2 Register (0x1E) 4 LSB bytes are for broken byte
	//size = size << 4;
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE((size & 0x0FF0) >> 4);
	BP_RFID_HW_PARALLEL_SIMPLE_WRITE((size & 0x000F) << 4);

	// this will be written to FIFO register 0x1F (increment mode)
	for (i = 0; i < size; ++i)
	{
		BP_RFID_HW_PARALLEL_SIMPLE_WRITE(data[i]);
		//printf("{%02x}", data[i]);
	}
	BP_RFID_HW_PARALLEL_STOP_MULTIPLE();

#else
#endif

	BP_RFID_HW_INT_ENABLE();

	timeout = BP_RFID_TIMEOUT;
	while ( !(TRF_IRQ_LAST_FLAGS & TRF_IRQ_TX) && (timeout) )
		;

	// clear flag
	TRF_IRQ_LAST_FLAGS &= ~TRF_IRQ_TX;

	printf("]\n");

	if ( timeout != 0 )
		return TRF_NO_ERROR;
	else
		return TRF_ERROR;
}

// TODO functions unchecked and mostly unused

void BP_RFID_TRF_Adjust_Gain()
{
	BP_RFID_TRF_Write_Command(TRF_CMD_ADJUST_GAIN);
}

void BP_RFID_TRF_Check_RF()
{
	BP_RFID_TRF_Write_Command(TRF_CMD_CHECK_INTERNAL_RF);
}

void BP_RFID_TRF_Check_AUX_RF()
{
	BP_RFID_TRF_Write_Command(TRF_CMD_CHECK_EXTERNAL_RF);
}

void BP_RFID_TRF_Initial_RF_Collision()
{
	BP_RFID_TRF_Write_Command(TRF_CMD_INITIAL_RF_COLLISION);
}

void BP_RFID_TRF_Response_RF_Collision_N()
{
	BP_RFID_TRF_Write_Command(TRF_CMD_RESPONSE_RF_COLLISION_N);
}
void BP_RFID_TRF_Initial_RF_Collision_0()
{
	BP_RFID_TRF_Write_Command(TRF_CMD_RESPONSE_RF_COLLISION_0);
}

void BP_RFID_TRF_Transmit_Next_Slot()
{
	BP_RFID_TRF_Write_Command(TRF_CMD_TRANSMIT_NEXT_SLOT);
}

void BP_RFID_TRF_Transmit_Close_Slot_Sequence()
{
	BP_RFID_TRF_Write_Command(TRF_CMD_CLOSE_SLOT_SEQUENCE);
}

// --- -----------------------------------------------------------------
//
//

/// Initializes BP hardware and software
///
/// TODO: Add hardware init to BP_HW
void BP_RFID_Init()
{
	int i;
	char regbuf[TRF_DEFAULT_REG_POWERON_COUNT + 1];

	TRF_IRQ_LAST_FLAGS = TRF_IRQ_SEMAPHORE = 0;

#ifdef BP_RFID_PARALLEL
	BP_RFID_HW_PARALLEL_INIT();
#else
	BP_RFID_HW_SERIAL_INIT();
#endif

	// Peripherals configured, now software

	IntEnable(INT_GPIOE);
	BP_RFID_HW_ENABLE();

	// Read default setting
	BP_RFID_TRF_Read_Registers(0, regbuf, TRF_DEFAULT_REG_POWERON_COUNT);

	// Check if registers are OK
	regbuf[TRF_DEFAULT_REG_POWERON_COUNT] = 1;
	for (i = 0; i < TRF_DEFAULT_REG_POWERON_COUNT; ++i)
		if (TRF_DEFAULT_REG_POWERON[i] != regbuf[i])
		{
			printf("[Def Reg mismatch @0x%02x (Def 0x%02x != Rx 0x%02x)]\n", i, TRF_DEFAULT_REG_POWERON[i], regbuf[i]);
			regbuf[TRF_DEFAULT_REG_POWERON_COUNT] = 0;
		}
	if (regbuf[TRF_DEFAULT_REG_POWERON_COUNT]) printf("[TRF default register settings OK]\n");

	BP_RFID_TRF_Write_Command(TRF_CMD_SOFT_INIT);
	BP_RFID_TRF_Idle();
	BP_RFID_TRF_Modulator_Control(TRF_MOD_OOK100);

	//BP_RFID_TRF_FIFO_Reset();
	// Set 3V operation.
	BP_RFID_TRF_Write_Register(TRF_REG_CHIP_STATE_CONTROL, 0);
	//BP_RFID_TRF_IRQ_Clear();

	//

	// Set LEDs
	BP_RFID_HW_LED1(LED_OFF);
	BP_RFID_HW_LED2(LED_OFF);
}

// --- IRQ support ----------------------------------------------------------------------------------------------

/// Function registers new callback for IRQ
///
void BP_RFID_Set_IRQ_Callback(void (*function)(char))
{
	BP_RFID_IRQ_Callback = function;
}

char BP_RFID_TRF_IRQ_Clear()
{
	return BP_RFID_TRF_Read_Register(TRF_REG_IRQ_STATUS);;
}

/// Waits for end of receive with timeout
///
char BP_RFID_TRF_Wait_For_Rx_End(void)
{
	timeout = BP_RFID_TIMEOUT;

	// while there is no interrupt or timeout do nothing
	while ( !(TRF_IRQ_LAST_FLAGS & TRF_IRQ_RX) && (timeout) )
		;

	// clear flag
	TRF_IRQ_LAST_FLAGS &= ~TRF_IRQ_RX;

	if ( timeout != 0 )
		return TRF_NO_ERROR;
	else
		return TRF_ERROR;
}

/// Waits for end of transmit with timeout
///
char BP_RFID_TRF_Wait_For_Tx_End(void)
{
	timeout = BP_RFID_TIMEOUT;

	// while there is no interrupt or timeout do nothing
	while ( !(TRF_IRQ_LAST_FLAGS & TRF_IRQ_TX) && (timeout) )
		;

	// clear flag
	TRF_IRQ_LAST_FLAGS &= ~TRF_IRQ_TX;

	if ( timeout != 0 )
		return TRF_NO_ERROR;
	else
		return TRF_ERROR;
}

/// Decreases timeout value if set
/// Plug this under systick or timer interrupt
///
void BP_RFID_Tick()
{
	if (timeout > 0) --timeout;
}

/// IRQ input handler
///
void IRQ_ISR(void)
{
	char c;

	// read interrupt register
	TRF_IRQ_LAST_FLAGS = BP_RFID_TRF_IRQ_Clear();

	printf("\n[I,%08b]", TRF_IRQ_LAST_FLAGS);

	// IRQ happened - set semaphore
	TRF_IRQ_SEMAPHORE = 1;

	// TODO: What is that?
	if (TRF_IRQ_LAST_FLAGS == 0)
	{
		printf("[unknown fail]");
		BP_RFID_TRF_Reset_Decoders();
		BP_RFID_TRF_FIFO_Reset();
		//step = 0;
		return;
	}

	// Tx is finished now
	if (TRF_IRQ_LAST_FLAGS & TRF_IRQ_TX)
	{
		printf("[TxE]");
		//BP_RFID_TRF_FIFO_Reset();
	}

	// Rx is finished
	if (TRF_IRQ_LAST_FLAGS & TRF_IRQ_RX)
	{
		// Load fifo content into buffer
		BP_RFID_RX_BUFFER_COUNT = 0;
		while (c = BP_RFID_TRF_FIFO_How_Many_Bytes())
		{
#ifdef DBG_PRINT
			printf("(%d)", c);
#endif
			BP_RFID_TRF_Read_Registers(TRF_REG_FIFO, BP_RFID_RX_BUFFER, c);
			BP_RFID_RX_BUFFER_COUNT += c;
		}
		printf("[RxE(CNT:%d)(RSSI:%d)]",BP_RFID_RX_BUFFER_COUNT,BP_RFID_TRF_Get_RSSI());
		BP_RFID_TRF_FIFO_Reset();


		//for (i = 0; i < BP_RFID_RX_BUFFER_COUNT; ++i)
		//	printf("(%x)", BP_RFID_RX_BUFFER[i]);
	}

	// FIFO is low or high
	if (TRF_IRQ_LAST_FLAGS & TRF_IRQ_FIFO)
	{
		//TODO: implement!
	}


	BP_RFID_TRF_IRQ_Clear();
	// clear IRQ flag in ARM
	ROM_GPIOPinIntClear (GPIO_PORTE_BASE, IRQ_PIN);

	// Call protocol int callback
	if (BP_RFID_IRQ_Callback != 0) BP_RFID_IRQ_Callback(TRF_IRQ_LAST_FLAGS);

}
