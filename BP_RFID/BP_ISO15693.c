/*
 * BP_ISO15693.c
 *
 *  Created on: 09-02-2013
 *      Author: Maciej
 */

#include "BP_RFID.h"
#include "TRF797x.h"
#include "util/uartstdio.h"

//TODO: remove
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "util/uartstdio.h"

#include "BP_ISO15693.h"

void BP_ISO15693_IRQ(char reg);

volatile char BP_ISO15693_Interrupt = 0;

char UID[8];

// TODO: multiple modes
void BP_ISO15693_Init()
{
	printf("[ISO15693 Initialized]\n");
	BP_RFID_Register_Callback(BP_ISO15693_IRQ);
	BP_RFID_TRF_Software_Init();
	BP_RFID_TRF_Modulator_Control(MOD_OOK100);
	BP_RFID_TRF_Set_ISO(
			TRF_PROTOCOL_ISO15693 | TRF_PROTOCOL_ISO15693_High_Bit_Rate);
}

void BP_ISO15693_Inventory()
{
	int i;
	//char s;
	char buf[8];

	printf("[ISO15693 Inventory:\n");

	BP_ISO15693_Interrupt = 0;

	//TODO: check for active mode

	BP_RFID_TRF_Turn_RF_On();

	BP_RFID_TRF_FIFO_Reset();

	// This will be transmitted

	buf[0] = ISO15693_FL0_Data_rate_flag | ISO15693_FL0_Inventory_flag|ISO15693_FL2_Nb_slots_flag;	//Flags
	buf[1] = ISO15693_CMD_Inventory;																//Command
	buf[2] = 0x00; 																					//ISO mask length

	BP_RFID_TRF_Transmit(buf, 3);

	// Wait for transmit
	while (!BP_ISO15693_Interrupt)
		;
	BP_ISO15693_Interrupt=0;

	// Wait for receive
	while (!BP_ISO15693_Interrupt)
		;
	BP_ISO15693_Interrupt=0;

	printf("ID:");

	for (i = 9; i > 1; i--)
		printf("(%2x)", BP_RFID_BUFFER[i]);

}

// TODO: IRQ callback

void BP_ISO15693_Read_Single_Block(char addr)
{
	int i;
	char buf[8];

	BP_ISO15693_Interrupt = 0;

	//TODO: Anticollision n inventory
	UID[0] = 0xe0 ;
	UID[1] = 0x07 ;
	UID[2] = 0x80 ;
	UID[3] = 0x40 ;
	UID[4] = 0x06 ;
	UID[5] = 0x57 ;
	UID[6] = 0x64 ;
	UID[7] = 0x3e ;

	printf("\n\n[ISO15693 Read:\n");

	//TODO: check for active mode

	//BP_RFID_TRF_Turn_RF_On();
	BP_RFID_TRF_FIFO_Reset();

	// This will be transmitted
	buf[0] = ISO15693_FL0_Inventory_flag | ISO15693_FL1_Option_flag;	  //flags
	buf[1] = ISO15693_CMD_Read_Single_Block;  //command
	buf[2] = 0x05;								  //block

	BP_RFID_TRF_Transmit(buf, 3);

	// Wait for transmit
	while (!BP_ISO15693_Interrupt)
		;
	BP_ISO15693_Interrupt=0;

	// Wait for receive
	while (!BP_ISO15693_Interrupt)
		;
	BP_ISO15693_Interrupt=0;

	for (i = 0; i < 20; ++i)
		printf("(%x)", BP_RFID_BUFFER[i]);

}

void BP_ISO15693_Get_System_Information(char addr)
{
	int i;
	char buf[8];

	BP_ISO15693_Interrupt = 0;

	printf("\n\n[ISO15693 Sys Info:\n");

	//TODO: check for active mode

	//BP_RFID_TRF_Turn_RF_On();
	BP_RFID_TRF_FIFO_Reset();

	// This will be transmitted
	buf[0] = 0;
	buf[1] = ISO15693_CMD_Get_System_Information;
	//buf[2] = 0x00;

	BP_RFID_TRF_Transmit(buf, 2);

	// Wait for transmit
	while (!BP_ISO15693_Interrupt)
		;
	BP_ISO15693_Interrupt=0;

	// Wait for receive
	while (!BP_ISO15693_Interrupt)
		;
	BP_ISO15693_Interrupt=0;

	for (i = 0; i < 15; ++i)
		printf("(%x)", BP_RFID_BUFFER[i]);
}

void BP_ISO15693_IRQ(char reg)
{
	char s;
	if (reg & TRF_IRQ_NORESP) printf("No response interrupt");
	if (reg & TRF_IRQ_COL) printf("Collision error");
	if (reg & TRF_IRQ_ERR3) printf("RX framing or EOF error");
	if (reg & TRF_IRQ_ERR2) printf("RX parity error (ISO14443A)");
	if (reg & TRF_IRQ_ERR1) printf("RX CRC Error");
	if (reg & TRF_IRQ_FIFO)
		printf("FIFO is less than 4 (TX) or more than 8 (RX)");
	if (reg & TRF_IRQ_RX)
	{
		printf("IRQ Set due to end of RX");
		printf(", RSSI:%d/7]", BP_RFID_TRF_Get_RSSI());
		printf("(%d bytes)", s = BP_RFID_TRF_FIFO_How_Many_Bytes());

		BP_RFID_Read_Registers(TRF_REG_FIFO, BP_RFID_BUFFER, s);

	//	BP_RFID_TRF_Turn_RF_Off();

		if (BP_RFID_BUFFER[0] != 0) switch (BP_RFID_BUFFER[1])
		{
		//	case ISO15693_RES_OK:
		//		printf("[ISO15693_RES_OK]");
		//		break;
			case ISO15693_RES_Not_Supported:
				printf("[ISO15693_RES_Not_Supported]");
				break;
			case ISO15693_RES_Not_Recognised:
				printf("[ISO15693_RES_Not_Recognised]");
				break;
			case ISO15693_RES_Operation_Not_Supported:
				printf("[ISO15693_RES_Operation_Not_Supported]");
				break;
			case ISO15693_RES_Unknown_Error:
				printf("[ISO15693_RES_Unknown_Error]");
				break;
			case ISO15693_RES_Block_Not_Available:
				printf("[ISO15693_RES_Block_Not_Available]");
				break;
			case ISO15693_RES_Block_Already_Locked:
				printf("[ISO15693_RES_Block_Already_Locked]");
				break;
			case ISO15693_RES_Block_Already_Locked_Content:
				printf("[ISO15693_RES_Block_Already_Locked_Content]");
				break;
			case ISO15693_RES_Programming_Unsuccessful:
				printf("[ISO15693_RES_Programming_Unsuccessful]");
				break;
			case ISO15693_RES_Lock_Kill_Unsuccessful:
				printf("[ISO15693_RES_Lock_Kill_Unsuccessful]");
				break;
			}
		BP_RFID_TRF_FIFO_Reset();
	}
	if (reg & TRF_IRQ_TX) printf("IRQ Set due to end of TX");

	BP_ISO15693_Interrupt = 1;
}
