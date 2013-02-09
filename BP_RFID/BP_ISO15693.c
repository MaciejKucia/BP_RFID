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

// TODO: multiple modes
void BP_ISO15693_Init()
{
	printf("\[ISO15693 Initialized]\n");
	BP_RFID_TRF_Software_Init();
	BP_RFID_TRF_Modulator_Control(MOD_OOK100);
	BP_RFID_TRF_Set_ISO(ISO_PROTOCOL_2);
}


void BP_ISO15693_Inventory()
{
	int i;
	char s;
	char buf[8];

	printf("\[ISO15693 Inventory:\n");

	//TODO: check for active mode

	BP_RFID_TRF_Turn_RF_On();

	// This will be transmitted
	buf[0] = 0x26; // ISO request flags
	buf[1] = 0x01; // ISO inventory command
	buf[2] = 0x00; // ISO mask ??

	BP_RFID_TRF_Transmit(buf, 3);

	//TODO: WAIT FOR INT
	SysCtlDelay(2 * SysCtlClockGet() / 1000);

	printf("%d bytes in FIFO\n", s = BP_RFID_TRF_FIFO_How_Many_Bytes());

	BP_RFID_Read_Registers(0x1F, BP_RFID_BUFFER, s);

	printf("%d bytes in FIFO\n", BP_RFID_Read_Register(FIFO_CONTROL));

	printf("[RSSI:%d/7]\n", BP_RFID_TRF_Get_RSSI());

	BP_RFID_TRF_Turn_RF_Off();

	for (i = 0; i < s; ++i)
		printf("(%x)", BP_RFID_BUFFER[i]);

	BP_RFID_TRF_FIFO_Reset();
}

// TODO: IRQ callback

void BP_ISO15693_Read_Single_Block(char addr)
{
}

void BP_ISO15693_Get_System_Information(char addr)
{
}

