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

void BP_RFID_NFC_Init()
{
	printf("[NFC Initialized]\n");
	BP_RFID_TRF_Software_Init();
	BP_RFID_TRF_Modulator_Control(MOD_OOK100);
	BP_RFID_TRF_Set_ISO(0);
}

void BP_RFID_NFC_Collision_Avoidance()
{

}


char BP_RFID_NFC_Send(char* data, char size)
{
	return 0;
}

