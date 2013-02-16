/*
 * BP_NDEF.c
 *
 *  Created on: 13-02-2013
 *      Author: Maciej
 */

#include "BP_RFID_TRF.h"
#include "TRF797x.h"
#include "util/uartstdio.h"

//TODO: remove
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "util/uartstdio.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
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

void BP_RFID_NDEF_IRQ(char reg);

volatile char BP_NDEF_Interrupt = 0;

char BP_RFID_TRF_Wait_For_FieldChange(void);

void SENS_RES(void)
{
#define CMD_SENSF_RES 0x01

	char packet[20] =
	{ 0x14, CMD_SENSF_RES, 0x01, 0xFE, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0, 0, 0, 0x0F, 0xAB };
	printf("[<<SENSF_RES]\n");
	BP_RFID_TRF_Transmit(packet, 20);
}

void ATR_RES(void)
{
	char packet[28] =
	{ 0x1C, 0xD5, 0x01, 0x01, 0xFE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0E, 0x32, 0x46, 0x66, 0x6D, 0x01, 0x01, 0x10, 0x03, 0x02, 0xFF, 0xFF };
	printf("[<<ATR_RES]\n");
	BP_RFID_TRF_Transmit(packet, 28);
}

void BP_RFID_NDEF_Process_Packet()
{
	//char i;

	//GPIOPinIntDisable(GPIO_PORTE_BASE, IRQ_PIN);

	//SysCtlDelay(1000); // TODO: why fault ISR without this?
	BP_RFID_HW_INT_DISABLE();
	printf("[Process_Packet:%d bytes", BP_RFID_RX_BUFFER[0]);



	// byte 0 is size

	// byte 1 is command

	switch (BP_RFID_RX_BUFFER[0])
	{
	// check packets with size of 6 bytes
	case 0x06:

		// is it SENSF_REQ?
		if (BP_RFID_RX_BUFFER[1] == 0x00)
		{
			printf("[>>SENSF_REQ]");
			// respond!
			SENS_RES();
		}
		break;

	case 0x1E:
		// is it ATR_REQ?
//		if ((BP_RFID_RX_BUFFER[1] == 0xD4) && (BP_RFID_RX_BUFFER[2] == 0x00))
//		{
//			printf("[>>ATR_REQ]\n");
//			ATR_RES();
//		}
		break;

	default:
		printf("(default)");

	}
	BP_RFID_RX_BUFFER_COUNT = 0;
	//GPIOPinIntEnable(GPIO_PORTE_BASE, IRQ_PIN);
	BP_RFID_HW_INT_ENABLE();
	printf("]\n");
}


void BP_NDEF_Init()
{
	printf("[NDEF Init ");

	BP_RFID_Set_IRQ_Callback(BP_RFID_NDEF_IRQ);

	BP_RFID_TRF_Software_Init();

	BP_RFID_TRF_Modulator_Control(TRF_MOD_OOK100);

	BP_RFID_TRF_Set_ISO(
	TRF_PROTOCOL_NFC_MODE_424kbps);

	// TRF_NFC_RX_SPEC_Gain_Reduction_15dB
	// TRF_NFC_RX_SPEC_848kbps_ISO14443
	BP_RFID_TRF_Write_Register(TRF_REG_RX_SPECIAL_SETTINGS, 0x1C); // 0001 1100

	// TRF_NFC_Target_Detection_Level_480mV
	BP_RFID_TRF_Write_Register(TRF_REG_NFC_Target_Det_Lvl, TRF_NFC_Field_Detection_Level_220mV);

	BP_RFID_TRF_Set_NFCID1(
			10,
			0x01,
			0xFE,
			0x26,
			0x03,
			0x19,
			0x90,
			0xAB,
			0xCD,
			0xEF,
			0xCD);

	// TRF_NFC_Field_Detection_Level_480mV
	BP_RFID_TRF_Write_Register(TRF_REG_NFC_Low_Field, TRF_NFC_Field_Detection_Level_220mV);

	//TRF_FIFO_TX_Level_32 | TRF_FIFO_RX_Level_96
	BP_RFID_TRF_Write_Register(TRF_REG_FIFO_Levels, 0x0F);

	BP_RFID_BUFFER_CLEAR();
	BP_RFID_TRF_FIFO_Reset();

	BP_RFID_TRF_Turn_RF_On();

	BP_RFID_TRF_Reset_Decoders();

	printf("OK]\n");

	// First get into range
	BP_RFID_TRF_Wait_For_FieldChange();
	printf("[In field]\n");

	for (;;)
	{
		// Wait for some packet
		if (BP_RFID_TRF_Wait_For_Rx_End())
			printf("[error on packet receive]\n");
		else
			// Process it!
			BP_RFID_NDEF_Process_Packet();




		//SENS_RES();
		//BP_RFID_TRF_Reset_Decoders();
	}

}


void BP_RFID_NDEF_IRQ(char flags)
{
	printf("{NDEF}");
	//char s;

	//printf("nf");

//	if (flags & TRF_IRQ_TX)
//	{
//		printf("[end TX]\n");
//		//BP_RFID_TRF_FIFO_Reset();
//		//BP_NDEF_Interrupt = 1;
//	}

//	if (flags & TRF_IRQ_RX)
//	{
//
//	} Rx is handled globally

	if (flags & TRF_IRQ_FIFO) printf("Signals the FIFO is 1/3 > FIFO > 2/3");

	if (flags & TRF_IRQ_NFC_Protocol_Error)
	{
		printf("[P error]");
		BP_RFID_RX_BUFFER_COUNT = 0;

		BP_RFID_TRF_Stop_Decoders();
		BP_RFID_TRF_Run_Decoders();
		BP_RFID_TRF_FIFO_Reset();
		BP_RFID_TRF_Read_Register(TRF_REG_IRQ_STATUS); //?

		// remove Rx flag
		TRF_IRQ_LAST_FLAGS = TRF_IRQ_NFC_Protocol_Error;
	}

	if (flags & TRF_IRQ_NFC_SDD_Finished) printf("SDD finished");

	if (flags & TRF_IRQ_NFC_RF_Field_Change)
		printf(
				"[RF fc %x]",
				BP_RFID_TRF_Read_Register(TRF_REG_NFC_Target_Protocol));

	if (flags & TRF_IRQ_NFC_Col_Avoid_Finished)
		printf("RF collision avoidance finished");

	if (flags & TRF_IRQ_NFC_Col_Avoid_Failed)
		printf("RF collision avoidance not finished successfully");
}

char BP_RFID_TRF_Wait_For_FieldChange(void)
{
	TRF_IRQ_SEMAPHORE=0;
	while(TRF_IRQ_SEMAPHORE==0);
	if (TRF_IRQ_LAST_FLAGS & TRF_IRQ_NFC_RF_Field_Change) return 0;
	return 1;
}
