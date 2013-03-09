/*
 * BP_ISO15693.c
 *
 *  Created on: 09-02-2013
 *      Author: Maciej
 */

#include "util/uartstdio.h"

#include "BP_RFID_TRF.h"
#include "TRF797x.h"
#include "BP_ISO15693.h"

void BP_ISO15693_IRQ(char);
void BP_ISO15693_Read_Single_Block(char);

void BP_ISO15693_IRQ(char reg)
{
	char c;
	if (reg & TRF_IRQ_NORESP) printf("No response interrupt");
	if (reg & TRF_IRQ_COL)
	{
		printf(
				"[C %02x]",
				c = BP_RFID_TRF_Read_Register(TRF_REG_COLLISION_POSITION));

		BP_RFID_RX_BUFFER_COUNT = 0;

		while (c = BP_RFID_TRF_FIFO_How_Many_Bytes())
		{
			//printf("r(%d)", c);
			BP_RFID_TRF_Read_Registers(TRF_REG_FIFO, BP_RFID_RX_BUFFER, c);
			BP_RFID_RX_BUFFER_COUNT += c;
		}
		//BP_RFID_TRF_Response_RF_Collision_N();
		//BP_RFID_TRF_Reset_Decoders();
		//BP_RFID_TRF_Transmit_Next_Slot();
		//BP_RFID_TRF_IRQ_Clear();
		BP_RFID_TRF_FIFO_Reset();
	}
	if (reg & TRF_IRQ_ERR3) printf("RX framing or EOF error");
	if (reg & TRF_IRQ_ERR2) printf("RX parity error (ISO14443A)");
	if (reg & TRF_IRQ_ERR1) printf("RX CRC Error");
//
//	if (reg & TRF_IRQ_RX) printf("[RxE]");
//
//	if (reg & TRF_IRQ_TX) printf("[TxE]");

}

void BP_ISO15693_Init()
{
	int i;
	char buf[12];

	printf("\n\n[ISO15693]\n");

	BP_RFID_Set_IRQ_Callback(BP_ISO15693_IRQ);

	BP_RFID_TRF_Write_Command(TRF_CMD_SOFT_INIT);

	BP_RFID_TRF_Modulator_Control(TRF_MOD_OOK100);

	BP_RFID_TRF_Set_ISO(
			TRF_PROTOCOL_ISO15693 | TRF_PROTOCOL_ISO15693_High_Bit_Rate);

	//BP_RFID_TRF_Write_Register(TRF_REG_RX_NO_RESPONSE_WAIT_TIME, 0x13);

	BP_RFID_TRF_Turn_RF_On();
	BP_RFID_TRF_Reset_Decoders();
	BP_RFID_TRF_FIFO_Reset();

	printf("[ISO15693 Inventory]\n");

	buf[0] = ISO15693_FL0_Data_rate_flag | ISO15693_FL0_Inventory_flag | ISO15693_FL2_Nb_slots_flag;
	buf[1] = ISO15693_CMD_Inventory;
	buf[2] = 0;
	BP_RFID_TRF_Transmit(buf, 3);

	BP_RFID_TRF_Wait_For_Rx_End();
	for (i = 0; i < BP_RFID_RX_BUFFER_COUNT; ++i)
		printf("%02x", BP_RFID_RX_BUFFER[i]);
	BP_RFID_RX_BUFFER_COUNT = 0;
	printf("!\n");

	buf[0] = ISO15693_FL0_Data_rate_flag;
	buf[1] = ISO15693_CMD_Read_Single_Block;
	buf[2] = 0;
	BP_RFID_TRF_Transmit(buf, 3);

	BP_RFID_TRF_Wait_For_Rx_End();
	for (i = 0; i < BP_RFID_RX_BUFFER_COUNT; ++i)
		printf("%02x", BP_RFID_RX_BUFFER[i]);
	BP_RFID_RX_BUFFER_COUNT = 0;
	printf("!");

	// if something received
//	if (!)
//	{
//		printf("!D:");
//
//
//		printf("[ISO15693 Read]\n");
//		buf[0] = 0;
//		buf[1] = ISO15693_CMD_Read_Single_Block;
//		buf[2] = 0x04; 									//ISO mask length
//
//		BP_RFID_TRF_Transmit(buf, 3);
//
//		if (!BP_RFID_TRF_Wait_For_Rx_End())
//		{
//			printf("!");
//		}
	//}
}
