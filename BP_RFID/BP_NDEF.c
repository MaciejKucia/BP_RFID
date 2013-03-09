/*
 * BP_NDEF.c
 *
 * Passive tag emulation
 *
 * NFC-DEP protocol
 *
 * Target sends NDEF URL
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
#include "BP_NDEF.h"

void BP_RFID_NDEF_IRQ(char reg);

char BP_RFID_TRF_Wait_For_FieldChange(void);

char NFCID[10] =
{ 0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A };

volatile char field = 0;
char step = 0;

void SYMM(void)
{
	//char i;
#define SYMM_SIZE 6

	char packet[SYMM_SIZE] =
	{ SYMM_SIZE, 							//[0]		size
			NFC_DEP_DEP_REQ0, NFC_DEP_DEP_REQ1, //[1:2]		command
			0, 									//[3]		DID !!
			0, 									//[4]		BRS bitrate max
			0									//[5]		FSL 64
	};

	BP_RFID_TRF_Transmit(packet, SYMM_SIZE);
	printf("[<<SYMM]\n");
	++step;
}

void SENSF_RES(void)
{
#define SENSF_RES_SIZE 20
	char packet[SENSF_RES_SIZE] =
	{ SENSF_RES_SIZE,										//[0]		size
			0x01, 									//[1] 		CMD SENSF_RES
			0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,	//[2:9]		NFCID2
			0xFF, 0xFF,  					//[10:11]	Requirements 84: PAD0
			0x01, 0x02, 0x03,									//[12:14]	PAD1
			0x00,										//[15] 		MRTIc [n]
			0x00,										//[16] 		MRTIu [n]
			0x00, 										//[17]		PAD2  [n]
			0x0F, 0xAB 								//[18:19]  Request data
	};

	if (BP_RFID_RX_BUFFER[4] == 0) packet[0] = 18;

//	// Copy Target NFC ID
//	for (i = 0; i < 10; ++i)
//	{
//		packet[3 + i] = Target_NFCID[i];
////		printf("{%02x}", packet[3 + i]);
//	}

	//SysCtlDelay(4*SysCtlClockGet()/1000);

	if (BP_RFID_RX_BUFFER[4] == 0)
		BP_RFID_TRF_Transmit(packet, 18);
	else
		BP_RFID_TRF_Transmit(packet, SENSF_RES_SIZE);

	printf("[<<SENSF_RES]\n");
}

void ATR_RES(void)
{
	char i;
#define ATR_RES_SIZE 31

	char packet[ATR_RES_SIZE] =
	{
			ATR_RES_SIZE, 						//[0]		size
			NFC_DEP_ATR_RES0,
			NFC_DEP_ATR_RES1, //[1:2]		ATR_RES command
			0, 0, 0, 0, 0, 0, 0, 0, 0,
			0,  		//[3:12]	NFC ID (target)
			0x00,							//[13] 		DID (target) device id
			0,								//[14]		BS (target) bitrate send
			0,							//[15]		BR (target) bitrate receive
			0x0E,     							//[16]		TO LLCP
			0x32,		//[17]		PP (target) Presence of Optional Parameters
			0x46, 0x66, 0x6D, 0x01, 0x01, 0x10, 0x03, 0x02, 0xFF, 0xFF, 0x04,
			0x01, 0x96
	//	,0x03, 0x02, 0xFF, 0xFF 			//[18+]
	};

	packet[14] = BP_RFID_RX_BUFFER[14];
	packet[15] = BP_RFID_RX_BUFFER[15];

	// Copy DID
	packet[13] = BP_RFID_RX_BUFFER[13];

	// Copy Target NFC ID
	for (i = 0; i < 8; ++i)
	{
		packet[3 + i] = NFCID[i];
//		printf("{%02x}", packet[3 + i]);
	}
	//packet[3 + 8]=packet[3 + 9]=0xAB;

	SysCtlDelay(4 * SysCtlClockGet() / 1000); //4ms

	BP_RFID_TRF_Transmit(packet, ATR_RES_SIZE);

//	char packet[28] =
//	{ 0x1C, 0xD5, 0x01, 0x01, 0xFE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0E, 0x32, 0x46, 0x66, 0x6D, 0x01, 0x01, 0x10, 0x03, 0x02, 0xFF, 0xFF };

	printf("[<<ATR_RES]\n");
}

void PSL_REQ(void)
{
	//char i;
#define PSL_REQ_SIZE 6

	char packet[PSL_REQ_SIZE] =
	{ PSL_REQ_SIZE, 						//[0]		size
			NFC_DEP_PSL_REQ0, NFC_DEP_PSL_REQ1, //[1:2]		ATR_RES command
			0, 									//[3]		DID !!
			0, 									//[4]		BRS bitrate max
			0									//[5]		FSL 64
	};

	BP_RFID_TRF_Transmit(packet, PSL_REQ_SIZE);
	printf("[<<PSL_REQ]\n");
}

void DEP_REQ(void)
{
#define DEP_REQ_SIZE 18
	// no DID nor NAD
	char packet[DEP_REQ_SIZE] =
	{ DEP_REQ_SIZE, 						//[0]		size
			NFC_DEP_DEP_REQ0, NFC_DEP_DEP_REQ1,	//[1:2]		DEP_REQ
			0x00, 							//[3]		PFB Protocol Format Byte
			// DATA
			0xD1,//	NFC Forum Well Known Type
			0x01,								//	Record Type length
			10,									//	Payload length
			0x55,								//	URI record type "U"
			0x03,								//	http://
			'k', 'u', 'c', 'i', 'a', '.', 'n', 'e', 't' };

//#define DEP_REQ_SIZE 6
//	// no DID nor NAD
//	char packet[DEP_REQ_SIZE] =
//	{
//			DEP_REQ_SIZE, 						//[0]		size
//			NFC_DEP_DEP_REQ0, NFC_DEP_DEP_REQ1,	//[1:2]		DEP_REQ
//			0x00, 								//[3]		PFB Protocol Format Byte
//			0x00,
//			0x00
//
//	};

	BP_RFID_TRF_Transmit(packet, DEP_REQ_SIZE);
	printf("[<<DEP_REQ]\n");
}

void BP_RFID_NDEF_Process_Packet()
{
	char i;
	if (BP_RFID_RX_BUFFER_COUNT == 0) return;

	//GPIOPinIntDisable(GPIO_PORTE_BASE, IRQ_PIN);

	//SysCtlDelay(1000); // TODO: why fault ISR without this?
	BP_RFID_HW_INT_DISABLE();
	printf("[Process_Packet:%d bytes ", BP_RFID_RX_BUFFER[0]);

	for (i = 0; i < BP_RFID_RX_BUFFER[0]; ++i)
		printf("(%02X)", BP_RFID_RX_BUFFER[i]);

	// byte 0 is size

	// byte 1 is command

	switch (BP_RFID_RX_BUFFER[0])
	{
	// check packets with size of 6 bytes
	case 0x06:

		// is it SENSF_REQ?
		if ((step == 0) && (BP_RFID_RX_BUFFER[1] == 0x00))
		{
			printf("[>>SENSF_REQ]");
			// respond!
			SENSF_RES();
		}
		break;

	case 0x1E:
		// is it ATR_REQ?(step == 0) &&
		if ((step == 1) && (BP_RFID_RX_BUFFER[1] == NFC_DEP_ATR_REQ0) && (BP_RFID_RX_BUFFER[2] == NFC_DEP_ATR_REQ1))
		{
			printf("[>>ATR_REQ]\n");

			for (i = 0; i < 10; ++i)
				NFCID[i] = BP_RFID_RX_BUFFER[3 + i];

			ATR_RES();
			//PSL_REQ();
			//step++;
			step++;
		}
		break;

	case 0x17:
		// (CONNECT) PDU?
		if ((step == 2) && (BP_RFID_RX_BUFFER[1] == 0xD5) && (BP_RFID_RX_BUFFER[2] == 0x07))
		{
			printf("[>>CONNECT PDU]");
			step++;
		}

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

	//BP_RFID_TRF_Software_Init();

	BP_RFID_TRF_Modulator_Control(TRF_MOD_OOK100);

	BP_RFID_TRF_Set_ISO(
	TRF_PROTOCOL_NFC_MODE_424kbps);

	// TRF_NFC_RX_SPEC_Gain_Reduction_15dB
	// TRF_NFC_RX_SPEC_848kbps_ISO14443
	BP_RFID_TRF_Write_Register(TRF_REG_RX_SPECIAL_SETTINGS, 0x1C); // 0001 1100

	// TRF_NFC_Target_Detection_Level_480mV
	BP_RFID_TRF_Write_Register(
			TRF_REG_NFC_Target_Det_Lvl,
			TRF_NFC_Field_Detection_Level_220mV);

	// TODO: does it do anything?
	BP_RFID_TRF_Set_NFCID(10, NFCID);

	// TRF_NFC_Field_Detection_Level_480mV
	BP_RFID_TRF_Write_Register(
			TRF_REG_NFC_Low_Field,
			TRF_NFC_Field_Detection_Level_220mV);

	//TRF_FIFO_TX_Level_32 | TRF_FIFO_RX_Level_96
	BP_RFID_TRF_Write_Register(TRF_REG_FIFO_Levels, 0x0F);

	BP_RFID_TRF_FIFO_Reset();

	BP_RFID_TRF_Turn_RF_On();

	BP_RFID_TRF_Reset_Decoders();

	printf("OK]\n");

	// First get into range
	//BP_RFID_TRF_Wait_For_FieldChange();
	//printf("[In field]\n");

	for (;;)
	{
		printf("=== [s:%d] ===\n", step);

		// Wait for some packet
		if (BP_RFID_TRF_Wait_For_Rx_End())
		{
			//BP_RFID_TRF_NFC_Target_Protocol_DEBUG();
			//printf("[error on packet receive]\n");
			//else
			// Process it!
			BP_RFID_NDEF_Process_Packet();

			if (step == 2)
			{
				DEP_REQ();
			}
		}

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
}

void BP_RFID_NDEF_IRQ(char flags)
{
	printf("{NDEF}");
	char c;

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

		BP_RFID_TRF_Reset_Decoders();
		BP_RFID_TRF_FIFO_Reset();
		BP_RFID_TRF_Read_Register(TRF_REG_IRQ_STATUS); //?

		// remove Rx flag
		TRF_IRQ_LAST_FLAGS = TRF_IRQ_NFC_Protocol_Error;
	}

	if (flags & TRF_IRQ_NFC_SDD_Finished) printf("SDD finished");

	if (flags & TRF_IRQ_NFC_RF_Field_Change)
	{
		c = BP_RFID_TRF_Read_Register(TRF_REG_NFC_Target_Protocol);
		//printf("[#####field %08b]", c);

		if ((c & TRF_NFC_Target_Protocol_RF_Level_collision)!= field)
		{
			printf("[Field change]");
			field = c & TRF_NFC_Target_Protocol_RF_Level_collision;

			// Disconnect - we need to start over
			if (field == 0)
			{
				step = 0;
				printf("END\n\n");

				BP_RFID_RX_BUFFER_COUNT = 0;

				BP_RFID_TRF_Reset_Decoders();
				BP_RFID_TRF_FIFO_Reset();
				BP_RFID_TRF_Read_Register(TRF_REG_IRQ_STATUS); //? TODO:
			}
		}
	}

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
	TRF_IRQ_SEMAPHORE = 0;
	while (TRF_IRQ_SEMAPHORE == 0)
		;
	if (TRF_IRQ_LAST_FLAGS & TRF_IRQ_NFC_RF_Field_Change) return 0;
	return 1;
}
