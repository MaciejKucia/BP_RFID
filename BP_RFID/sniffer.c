/*
 * BP_ISO15693.c
 *
 *  Created on: 09-02-2013
 *      Author: Maciej
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

#include "util/uartstdio.h"

#include "TRF797x.h"
#include "BP_RFID_TRF.h"


//#include "BP_ISO15693.h"




void BP_SNIFF()
{
	int i;
	//char s;
	char buf[8];

	printf("[Sniffer Initialized]\n");

		BP_RFID_Set_IRQ_Callback(0);

		//BP_RFID_TRF_Software_Init();
		BP_RFID_TRF_Modulator_Control(TRF_MOD_OOK100);
		BP_RFID_TRF_Set_ISO(
				TRF_PROTOCOL_ISO15693 | TRF_PROTOCOL_ISO15693_High_Bit_Rate | TRF_PROTOCOL_Dir_Mode1);

	BP_RFID_TRF_Turn_RF_On();
	BP_RFID_TRF_FIFO_Reset();
	BP_RFID_TRF_Stop_Decoders();

	//BP_RFID_TRF_Write_Register(BP_RFID_TRF_Read_Register(0) |TRF_CSR_DIRECT);

	buf[0] = BP_RFID_TRF_Read_Register(0) |TRF_CSR_DIRECT;

	BP_RFID_HW_PARALEL_SET_INPUT();

	BP_RFID_HW_PARALLEL_START();
	BP_RFID_HW_SIMPLE_WRITE((0x00 & 0x1F));
	BP_RFID_HW_SIMPLE_WRITE(buf[0]);

	for (;;)
	{
		 // clock
		//while (ROM_GPIOPinRead(GPIO_PORTC_BASE, T3_PIN)==0);
		while (ROM_GPIOPinRead(GPIO_PORTB_BASE, T2_PIN)==0);
		// data
		printf("%s",ROM_GPIOPinRead(GPIO_PORTB_BASE, T2_PIN)?"0":"1");
	}

}

