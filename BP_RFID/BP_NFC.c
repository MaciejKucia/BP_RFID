/*
 * BP_NFC.c
 *
 *  Created on: Feb 6, 2013
 *      Author: x0184343
 */

#include "BP_RFID.h"
#include "TRF797x.h"
#include "util/uartstdio.h"

//TODO: remove
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "util/uartstdio.h"

//NFC modes:
// target
// initiator

void BP_RFID_NFC_Init()
{
	printf("[NFC Initialized]\n");
	BP_RFID_TRF_Software_Init();
	BP_RFID_TRF_Modulator_Control(MOD_OOK100);
	// passive
	BP_RFID_TRF_Set_ISO(TRF_PROTOCOL_NFC_MODE_106kbps | TRF_PROTOCOL_Dir_Mode);
}

void BP_RFID_NFC_Collision_Avoidance()
{
	//char buf[8];
	int i;

	//480 mV
	BP_RFID_Write_Register(TRF_REG_NFC_Target_Det_Lvl,
			(1 << 6) | (1 << 5) | (1 << 1));

	//BP_RFID_Write_Register_Continous(TRF_REG_NFCID1_Number, );
	BP_RFID_BUFFER[0] = TRF_REG_NFCID1_Number | CONTINOUS_MODE;
	BP_RFID_BUFFER[1] = 0x12;
	BP_RFID_BUFFER[2] = 0x34;
	BP_RFID_BUFFER[3] = 0x56;
	BP_RFID_BUFFER[4] = 0x78;
	BP_RFID_HW_WRITE_PARALLEL_MULTIPLE(BP_RFID_BUFFER, 5);

	BP_RFID_Write_Register(TRF_REG_NFC_Low_Field, (1 << 1));

	for (i = 0; i < 255; ++i)
	{
		printf("%d[%x]\n",i, BP_RFID_Read_Register(TRF_REG_NFC_Target_Protocol));
		SysCtlDelay(5000000);
	}
	// BP_RFID_TRF_Initial_RF_Collision();

	//BP_RFID_TRF_Modulator_Control(MOD_);

	//INITIAL_RF_COLLISION

	// This will be transmitted
	//buf[0] = 't';
	//buf[1] = 'e';
	//buf[2] = 's';
	////buf[3] = 't';
	//buf[4] = 0;

	//BP_RFID_TRF_Transmit(buf, 5);

	// wait for int.
}

char BP_RFID_NFC_Send(char* data, char size)
{
	return 0;
}

