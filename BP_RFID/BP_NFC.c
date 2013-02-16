/*
 * BP_NFC.c
 *
 *  Created on: Feb 6, 2013
 *      Author: x0184343
 */

#include "BP_RFID_TRF.h"
#include "TRF797x.h"
#include "util/uartstdio.h"

//TODO: remove
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "util/uartstdio.h"

//NFC modes:
// target
// initiator

volatile char  BP_RFID_NFC_IRQ_RX_FLAG;
volatile char  BP_RFID_NFC_IRQ_TX_FLAG;

void BP_RFID_NFC_IRQ(char flags);

void BP_RFID_NFC_Init()
{
	printf("[NFC Initialized]\n");
	BP_RFID_Set_IRQ_Callback(BP_RFID_NFC_IRQ);
	BP_RFID_TRF_Software_Init();
	BP_RFID_TRF_Modulator_Control(TRF_MOD_OOK100);
	// passive
	BP_RFID_TRF_Set_ISO(TRF_PROTOCOL_NFC_MODE_EMU_ISO14443B);
	//BP_RFID_TRF_Set_ISO(TRF_PROTOCOL_NFC_MODE_106kbps);
}

void BP_RFID_NFC_Collision_Avoidance()
{
	char buffer[16];
	//int i;


	BP_RFID_TRF_Write_Register(TRF_REG_RX_SPECIAL_SETTINGS, 0x3C);
	//BP_RFID_Read_Register(TRF_REG_RX_SPECIAL_SETTINGS) | TRF_NFC_RX_SPEC_Gain_Reduction_15dB | TRF_NFC_RX_SPEC_ISO14443 | TRF_NFC_RX_SPEC_848kbps_ISO14443);
	//0x3C);

	BP_RFID_TRF_Write_Register(
			TRF_REG_NFC_Target_Det_Lvl,
			7);
			//TRF_NFC_Target_Detection_Level_170mW | TRF_NFC_Target_Detection_NFCID1_4);
	//	0x01 | 0x02 | 0x04 | (1 << 6) | (1 << 5));

	//BP_RFID_TRF_Set_NFCID1(7, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD);


	BP_RFID_TRF_Write_Register(TRF_REG_NFC_Low_Field, 3); //TODO

	BP_RFID_TRF_Write_Register(TRF_REG_ISO_14443B_OPTIONS, 0);

	// Start radio and sensing circuitry
	BP_RFID_TRF_Write_Register(
			TRF_REG_CHIP_STATE_CONTROL,
			TRF_CSR_RECEIVER_ON | TRF_CSR_RF_ON);

	BP_RFID_TRF_FIFO_Reset();
	BP_RFID_TRF_Stop_Decoders(); //Reset better?
	BP_RFID_TRF_Run_Decoders();

	BP_RFID_NFC_IRQ_RX_FLAG = 0;

	while(!BP_RFID_NFC_IRQ_RX_FLAG);

	BP_RFID_TRF_FIFO_Reset();
	BP_RFID_TRF_Stop_Decoders(); //Reset better?
	BP_RFID_TRF_Run_Decoders();

	//if (BP_RFID_BUFFER[0]==0x05)
	//{
		printf("[OK]");

      // ATQB Packet
//      buffer[0] = 12;         // Length
//      buffer[1] = 0x50;       // ATQB Start Byte
//      // NFCID 3:0
//      buffer[2] = 0x12;
//      buffer[3] = 0x34;
//      buffer[4] = 0x56;
//      buffer[5] = 0x78;
//
//      // Application Data Bytes
//      buffer[6] = 0x40;     // AFI
//      buffer[7] = 0xE2;     // CRC_B
//      buffer[8] = 0xAF;     // CRC_B
//      buffer[9] = 0x11;     // # of applications (1)
//
//      // Protocol Info Bytes
//      buffer[10] = 0x80;    // Date Rate Capability ( Only Support 106 kbps)
//      // Max Frame/Protocol type (128 bytes / PICC compliant to -4)
//      buffer[11] = 0x71;
//      // (FWI/ADC/FO) ( FWT = 77.3mSec, ADC = coded according to AFI, CID supported)
//      buffer[12] = 0x85;

      BP_RFID_NFC_IRQ_TX_FLAG = 0;

      buffer[0] = 0x01;         // Length
      buffer[1] = 0xFF;       // ATQB Start Byte


      BP_RFID_TRF_Transmit(buffer,2);

      while(!BP_RFID_NFC_IRQ_TX_FLAG);
	//}

	printf("[END]");
}

void BP_RFID_NFC_IRQ(char flags)
{
	char c;
	int i;
	char s;

	printf("NFC:");

	if (flags & TRF_IRQ_TX)
	{
		printf("IRQ set due to end of TX");
		BP_RFID_NFC_IRQ_TX_FLAG = 1;
		BP_RFID_TRF_FIFO_Reset();
	}

	if (flags & TRF_IRQ_RX)
	{
		printf("IRQ set due to RX start");

		printf("{%d bytes in FIFO]\n", s = BP_RFID_TRF_FIFO_How_Many_Bytes());
		BP_RFID_TRF_Read_Registers(TRF_REG_FIFO, BP_RFID_BUFFER, s);
		//printf("%d bytes in FIFO\n", BP_RFID_TRF_FIFO_How_Many_Bytes());
		for (i = 0; i < s; ++i)
			printf("(%x)", BP_RFID_BUFFER[i]);

		printf("{%d bytes in FIFO]\n", s = BP_RFID_TRF_FIFO_How_Many_Bytes());

		BP_RFID_TRF_FIFO_Reset();

		BP_RFID_NFC_IRQ_RX_FLAG=1;
	}

	if (flags & TRF_IRQ_FIFO)
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

char BP_RFID_NFC_Send(char* data, char size)
{
	return 0;
}

