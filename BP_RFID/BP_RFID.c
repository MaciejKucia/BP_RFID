/*
 * BP_RFID.c
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

#include "BP_RFID.h"

#include "util/uartstdio.h"

#include "TRF797x.h"

/// Values present in TRF7970A after power on
///
char DEFAULT_REG_POWERON[] =
{ 0x01, 0x21, 0x00, 0x00, 0xC1, 0xC1, 0x00, 0x0E, 0x07, 0x91, 0x10, 0x87, 0x00,
		0x3E, 0x00, 0x40 };

/// Main buffer
///
#define BP_RFID_BUFFER_SIZE 512
char BP_RFID_BUFFER[BP_RFID_BUFFER_SIZE];

/// Clear the buffer (mainly debug)
///
void BP_RFID_BUFFER_CLEAR(void)
{
	int i;
	for (i = 0; i < BP_RFID_BUFFER_SIZE; ++i)
		BP_RFID_BUFFER[i] = 0;
}

/// Writes one register
///
void BP_RFID_Write_Register(char adr, char data)
{
#ifdef BP_RFID_PARALLEL
	BP_RFID_HW_WRITE_PARALLEL(adr, data, 0);
#endif
}

/// Writes one command
///
void BP_RFID_Write_Command(char data)
{
#ifdef BP_RFID_PARALLEL
	BP_RFID_HW_WRITE_PARALLEL(data, 0, 1);
#endif
}

/// Writes multiple registers
///
//void BP_RFID_Write_Registers(char adr, char* data, char size)
//{
//#ifdef BP_RFID_PARALLEL
//	// TODO: add address
//	BP_RFID_HW_WRITE_PARALLEL_MULTIPLE(data, size);
//#endif
//}

/// Reads one register
///
char BP_RFID_Read_Register(char adr)
{
#ifdef BP_RFID_PARALLEL
	return BP_RFID_HW_READ_PARALLEL(adr);
#endif
}

/// Reads multiple registers
///
void BP_RFID_Read_Registers(char adr, char* buffer, char size)
{
#ifdef BP_RFID_PARALLEL
	BP_RFID_HW_READ_PARALLEL_MULTIPLE(adr, buffer, size);
#endif
}


// Soft commands
/*#define IDLE						0x00
#define SOFT_INIT					0x03
#define INITIAL_RF_COLLISION		0x04
#define RESPONSE_RF_COLLISION_N		0x05
#define RESPONSE_RF_COLLISION_0		0x06
#define	RESET						0x0F
#define TRANSMIT_NO_CRC				0x10
#define TRANSMIT_CRC				0x11
#define DELAY_TRANSMIT_NO_CRC		0x12
#define DELAY_TRANSMIT_CRC			0x13
#define TRANSMIT_NEXT_SLOT			0x14
#define CLOSE_SLOT_SEQUENCE			0x15
#define STOP_DECODERS				0x16
#define RUN_DECODERS				0x17
#define CHECK_INTERNAL_RF			0x18
#define CHECK_EXTERNAL_RF			0x19
#define ADJUST_GAIN					0x1A*/

void BP_RFID_TRF_Idle()
{
	BP_RFID_Write_Command(IDLE);
}

void BP_RFID_TRF_Software_Init()
{
	BP_RFID_BUFFER_CLEAR();
	BP_RFID_Write_Command(SOFT_INIT);
}

void BP_RFID_TRF_IRQ_Clear()
{
	BP_RFID_Write_Register(IRQ_STATUS,0x00);
}

void BP_RFID_TRF_Initial_RF_Collision()
{
	BP_RFID_Write_Command(INITIAL_RF_COLLISION);
}

void BP_RFID_TRF_Run_Decoders()
{
	BP_RFID_Write_Command(RUN_DECODERS);
}

void BP_RFID_TRF_Stop_Decoders()
{
	BP_RFID_Write_Command(STOP_DECODERS);
}

void BP_RFID_TRF_Transmit_Next_Slot()
{
	BP_RFID_Write_Command(TRANSMIT_NEXT_SLOT);
}

void BP_RFID_TRF_Turn_RF_Off()
{
	BP_RFID_LED1(LED_OFF);
	// turn RF_ON bit off
	char reg = BP_RFID_Read_Register(CHIP_STATE_CONTROL);
	BP_RFID_Write_Register(CHIP_STATE_CONTROL,reg&~(CSR_RF_ON));


	// No RF no interrupts TODO: check
	BP_RFID_TRF_IRQ_Clear();
}

void BP_RFID_TRF_Turn_RF_On()
{
	BP_RFID_LED1(LED_BLINK_FAST);
	// turn RF_ON bit on
	char reg = BP_RFID_Read_Register(CHIP_STATE_CONTROL);
	BP_RFID_Write_Register(CHIP_STATE_CONTROL,reg|(CSR_RF_ON));
}

void BP_RFID_TRF_Modulator_Control(char control)
{
	BP_RFID_Write_Register(MODULATOR_CONTROL, control);
}

char BP_RFID_TRF_Get_ISO(void)
{
	return BP_RFID_Read_Register(ISO_CONTROL);
}

void BP_RFID_TRF_Set_ISO(char iso)
{
	BP_RFID_Write_Register(ISO_CONTROL,iso);
}

char BP_RFID_TRF_Is_OSC_Stable(void)
{
	return (BP_RFID_Read_Register(RSSI_LEVELS)&(1<<6)?1:0);
}


char BP_RFID_TRF_Get_RSSI(void)
{
	return (BP_RFID_Read_Register(RSSI_LEVELS)&0x07);
}

char BP_RFID_TRF_Get_AUX_RSSI(void)
{
	return ((BP_RFID_Read_Register(RSSI_LEVELS)&0x38)>>3);
}

void BP_RFID_TRF_FIFO_Reset(void)
{
	BP_RFID_Write_Command(RESET);
}

char BP_RFID_TRF_FIFO_How_Many_Bytes(void)
{
	return BP_RFID_Read_Register(FIFO_CONTROL)&0x7F;
}

/// Is FIFO overflowed
///
char BP_RFID_TRF_FIFO_Is_OVF(void)
{
	return (BP_RFID_Read_Register(FIFO_CONTROL)&(1<<7)?1:0);
}

//char BP_RFID_TRF_FIFO_Read_All(char * buff)
//{
//	char i;
//	for(i=0;i<BP_RFID_TRF_FIFO_How_Many_Bytes;++i)
//		buff[i] =
//
//}


// TODO: implement serial
void BP_RFID_Init()
{
	int i;

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

	// Set LEDs
	BP_RFID_LED1(LED_ON);
	BP_RFID_LED2(LED_ON);

	// Reset
	BP_RFID_HW_DISABLE();
	BP_RFID_HW_ENABLE();

	printf("[RFID: parallel interface active]\n");

	// Check if registers are OK
	BP_RFID_HW_READ_PARALLEL_MULTIPLE(0, BP_RFID_BUFFER, 16);
	for (i = 0; i < 16; ++i)
		if (DEFAULT_REG_POWERON[i] != BP_RFID_BUFFER[i])
			printf(
					"[Warning! Default register setting mismatch @0x%X (%x != %x)]\n",
					i, DEFAULT_REG_POWERON[i], BP_RFID_BUFFER[i]);


	BP_RFID_TRF_Software_Init();
	BP_RFID_TRF_Idle();
	BP_RFID_TRF_Modulator_Control(MOD_OOK100);

	// Write some configuration
	//
	// B7-B0: Chip State
	// 0 Active mode
	// 0 Direct mode 2
	// 1 RF output active
	// 0 full output power
	// 0 Main RX input
	// 0 AGC off
	// 0 Automatic Enable
	// 0 3v3 operation
	//BP_RFID_Write_Register(CHIP_STATE_CONTROL, (1 << 5));
	//
	// B7-B0 ISO Control
	// 0 CRC
	// 0 Direct Mode 0
	// 0 RFID Mode
	// ]
	// ] ISO protocol
	// ] 0
	// ]
	//BP_RFID_Write_Register(ISO_CONTROL, ISO_PROTOCOL_2);
	//
	// Regulator and I/O Control
	// 1 - Automatic setting
	// 0 - no external power amp
	// 0 - normal periph comm voltage
	// 0 ]
	// 0 ] unused
	// 0 }
	// 0 } 400 mV difference automatic regulator setting
	//BP_RFID_Write_Register(REGULATOR_CONTROL, (1 << 7));

	// Modulator and SYS_CLK Control
	// 0 - default operation
	// 0]
	// 0] no sysclk output
	// 0 - default
	// 0}
	// 0}
	// 0} ASK 10%
	// 0}
	//BP_RFID_Write_Register(MODULATOR_CONTROL, 0);

	//BP_RFID_Write_Register(IRQ_STATUS			,(1<<7));

	// Set LEDs
	BP_RFID_LED1(LED_OFF);
	BP_RFID_LED2(LED_OFF);

#endif
}



/// IRQ input handler
///
// TODO: implement error handling
void IRQ_ISR(void)
{
	// read int register and clear it
	char reg = BP_RFID_Read_Register(IRQ_STATUS);
	BP_RFID_Write_Register(IRQ_STATUS,0x00);

	printf("[INTERRUPT:");

	if (reg&IRQ_NORESP) printf("No response interrupt");
	if (reg&IRQ_COL) printf("Collision error");
	if (reg&IRQ_ERR3) printf("RX framing or EOF error");
	if (reg&IRQ_ERR2) printf("RX parity error (ISO14443A)");
	if (reg&IRQ_ERR1) printf("RX CRC Error");
	if (reg&IRQ_FIFO) printf("FIFO is less than 4 (TX) or more than 8 (RX)");
	if (reg&IRQ_RX) printf("IRQ Set due to end of RX");
	if (reg&IRQ_TX) printf("IRQ Set due to end of TX");

	printf("]\n");

	GPIOPinIntClear(GPIO_PORTE_BASE, IRQ_PIN);
}

// NFC handled differently
void IRQ_ISR_NFC(void)
{

}
