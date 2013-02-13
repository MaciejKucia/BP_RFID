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

void BP_RFID_NDEF_IRQ(char reg);

volatile char BP_NDEF_Interrupt = 0;

void BP_NDEF_Init()
{
	printf("[NDEF Initialization...]\n");

	BP_RFID_Set_IRQ_Callback(BP_RFID_NDEF_IRQ);

	BP_RFID_TRF_Software_Init();

	BP_RFID_TRF_Modulator_Control(TRF_MOD_OOK100);

	BP_RFID_TRF_Set_ISO(
	TRF_PROTOCOL_NFC_MODE_424kbps);

	// TRF_NFC_RX_SPEC_Gain_Reduction_15dB
	// TRF_NFC_RX_SPEC_848kbps_ISO14443
	BP_RFID_TRF_Write_Register(TRF_REG_RX_SPECIAL_SETTINGS, 0x1C); // 0001 1100

	// TRF_NFC_Target_Detection_Level_480mV
	BP_RFID_TRF_Write_Register(TRF_REG_NFC_Target_Det_Lvl, 0x01);

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
	BP_RFID_TRF_Write_Register(TRF_REG_NFC_Low_Field, 0x01);

	//TRF_FIFO_TX_Level_32 | TRF_FIFO_RX_Level_96
	BP_RFID_TRF_Write_Register(TRF_REG_FIFO_Levels, 0x0F);

	BP_RFID_BUFFER_CLEAR();
	BP_RFID_TRF_FIFO_Reset();

	BP_RFID_TRF_Turn_RF_On();

	BP_RFID_TRF_Reset_Decoders();

	printf("[NDEF Initialized]\n");
}

void SENS_RES(void)
{
#define CMD_SENSF_RES 0x01

	char packet[20] =
	{ 0x14, CMD_SENSF_RES, 0x01, 0xFE, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0, 0, 0, 0x0F, 0xAB };
	printf("[<<SENSF_RES]\n");
	BP_RFID_TRF_Transmit(packet, 20);
}

void BP_RFID_NDEF_Process_Packet()
{
	char i;

	for (i = 0; i < BP_RFID_BUFFER[0]; ++i)
		printf("(%x)", BP_RFID_BUFFER[i]);

	printf("[BP_RFID_NDEF_Process_Packet:%d bytes]\n", BP_RFID_BUFFER[0]);

	// byte 0 is size

	// byte 1 is command

	switch (BP_RFID_BUFFER[0])
	{
	// check packets with size of 6 bytes
	case 6:

		// is it SENSF_REQ?
		if (BP_RFID_BUFFER[1] == 0x00)
		{
			printf("[>>SENSF_REQ]\n");
			// respond!
			SENS_RES();
		}
		break;

	case 30:
		// is it ATR_REQ?
		if ((BP_RFID_BUFFER[1] == 0xD4) && (BP_RFID_BUFFER[2] == 0x00))
		{
			printf("[>>ATR_REQ]\n");
			// TODO
		}
		break;

	case 0xD4:
		//if(BP_RFID_BUFFER[2] != 0) printf("ERROR!\n");

		//printf("[>>ATR_REQ]\n");

		break;

	default:
		printf("(default)\n");

	}
}

void BP_RFID_NDEF_IRQ(char flags)
{
	char c;
	char s;

	printf("NDEF:");

	if (flags & TRF_IRQ_NFC_Tx_End)
	{
		printf("IRQ set due to end of TX");
		BP_NDEF_Interrupt = 1;
		BP_RFID_TRF_FIFO_Reset();
	}

	if (flags & TRF_IRQ_NFC_Rx_Start)
	{
		printf("IRQ set due to RX start\n");

		//BP_RFID_TRF_NFC_Target_Protocol_DEBUG();

		for (;;)
		{
			printf(
					"[%d bytes in FIFO]\n",
					s = BP_RFID_TRF_FIFO_How_Many_Bytes());
			BP_RFID_TRF_Read_Registers(TRF_REG_FIFO, BP_RFID_BUFFER, s);

			BP_RFID_NDEF_Process_Packet();
			BP_RFID_BUFFER_CLEAR();

			//for (i = 0; i < s; ++i)	printf("(%x)", BP_RFID_BUFFER[i]);

			printf(
					"[still %d bytes in FIFO]\n",
					s = BP_RFID_TRF_FIFO_How_Many_Bytes());

			if (s == 0) break; // no more bytes to read
		}
		BP_RFID_TRF_FIFO_Reset();

		BP_NDEF_Interrupt = 1;

	}

	if (flags & TRF_IRQ_NFC_FIFO_High)
		printf("Signals the FIFO is 1/3 > FIFO > 2/3");

	if (flags & TRF_IRQ_NFC_Protocol_Error) printf("Protocol error");

	if (flags & TRF_IRQ_NFC_SDD_Finished) printf("SDD finished");

	if (flags & TRF_IRQ_NFC_RF_Field_Change)
	{

		c = BP_RFID_TRF_Read_Register(TRF_REG_NFC_Target_Det_Lvl) & 0xDF;
		printf("RF field change %x", c);

		printf(c & TRF_NFC_Target_Protocol_RF_Level_wake ? "(in)" : "(out)");
		//c &= ~(TRF_NFC_Target_Detection_SDD_Enabled);
		//BP_RFID_Write_Register(TRF_REG_NFC_Target_Det_Lvl, c);

		//TODO: reset when out of field?
	}

	if (flags & TRF_IRQ_NFC_Col_Avoid_Finished)
		printf("RF collision avoidance finished");

	if (flags & TRF_IRQ_NFC_Col_Avoid_Failed)
		printf("RF collision avoidance not finished successfully");
}
