/*
 * BP_ISO15693.h
 *
 *  Created on: 09-02-2013
 *      Author: Maciej
 */

#ifndef BP_ISO15693_H_
#define BP_ISO15693_H_

// ISO/IEC JTC1/SC17 N 3316

// Commands
#define ISO15693_CMD_Inventory 							 0x01
#define ISO15693_CMD_Stay_Quiet 						 0x02
#define ISO15693_CMD_Read_Single_Block					 0x20
#define ISO15693_CMD_Write_Single_Block 				 0x21 //Flags
#define ISO15693_CMD_Lock_Block  						 0x22 //Flags
#define ISO15693_CMD_Read_Multiple_Blocks  				 0x23
#define ISO15693_CMD_Write_Multiple_Blocks  			 0x24 //Flags
#define ISO15693_CMD_Select  							 0x25
#define ISO15693_CMD_Reset_to_Ready  					 0x26
#define ISO15693_CMD_Write_AFI  						 0x27 //Flags
#define ISO15693_CMD_Lock_AFI  							 0x28 //Flags
#define ISO15693_CMD_Write_DSFID 						 0x29 //Flags
#define ISO15693_CMD_Lock_DSFID 						 0x2A
#define ISO15693_CMD_Get_System_Information 			 0x2B
#define ISO15693_CMD_Get_Multiple_Block_Security_Status  0x2C

// Responses
#define ISO15693_RES_OK 							 	 0x00
#define ISO15693_RES_Not_Supported 						 0x01
#define ISO15693_RES_Not_Recognised 					 0x02
#define ISO15693_RES_Operation_Not_Supported 			 0x03
#define ISO15693_RES_Unknown_Error						 0x0F
#define ISO15693_RES_Block_Not_Available				 0x10
#define ISO15693_RES_Block_Already_Locked				 0x11
#define ISO15693_RES_Block_Already_Locked_Content		 0x12
#define ISO15693_RES_Programming_Unsuccessful			 0x13
#define ISO15693_RES_Lock_Kill_Unsuccessful				 0x14

// Further ones are custom A0-DF


//Flags


/// FL0 Request

//b1 Sub-carrier_flag
//1 Two sub-carriers shall be used by the VICC
//0 A single sub-carrier frequency shall be used by the VICC
#define ISO15693_FL0_Sub_carrier_flag 					(1<<0) //01

//b2 Data_rate_flag
//1 High data rate shall be used
//0 Low data rate shall be used
#define ISO15693_FL0_Data_rate_flag						(1<<1) //02

//b3 Inventory_flag
//1 Flags 5 to 8 meaning is according to table 5
//0 Flags 5 to 8 meaning is according to table 4
#define ISO15693_FL0_Inventory_flag						(1<<2) //04

//b4 Protocol
//Extension_flag 1 Protocol format is extended. Reserved for future use
//0 No protocol format extension
#define ISO15693_FL0_Protocol_Extension_flag			(1<<3) //08

/// FL1 Inventory not set

//b5 Select_flag
//0 Request shall be executed by any VICC according to the setting of Address_flag
//1 Request shall be executed only by VICC in selected state. The Address_flag shall be set to 0 and the UID field shall not be included in the request.
#define ISO15693_FL1_Select_flag						(1<<4) //10

//b6 Address_flag
//0 Request is not addressed. UID field is not included. It shall be executed by any VICC.
//1 //Request is addressed. UID field is included. It shall be executed only by the VICC whose UID matches the UID specified in the request.
#define ISO15693_FL1_Address_flag						(1<<5) //20

//b7 Option_flag be set to 0 if not otherwise defined by the command.
//0 Meaning is defined by the command description. It shall be set to 0 if not otherwise defined by the command.
//1 Meaning is defined by the command description.
#define ISO15693_FL1_Option_flag						(1<<6) //40

//b8 RFU - reserved for future use
//#define ISO15693_FL1_RFU								(1<<7)


/// FL2 Inventory set

//b5 AFI_flag
//0 AFI field is not present
//1 AFI field is present
#define ISO15693_FL2_AFI_flag							(1<<4) //10

//b6 Nb_slots_flag
//0 16 slots
//1 1 slot
#define ISO15693_FL2_Nb_slots_flag						(1<<5) //20

//b7 Option_flag be set to 0 if not otherwise defined by the command.
//0 Meaning is defined by the command description. It shall be set to 0 if not otherwise defined by the command.
//1 Meaning is defined by the command description.
#define ISO15693_FL2_Option_flag						(1<<6) //40

//b8 RFU 0
//#define ISO15693_FL2_RFU								(1<<7)


/// FL3 Response

//b1 Error_flag
//0 No error
//1 Error detected. Error code is in the "Error" field.
#define ISO15693_FL3_Error_flag							(1<<0)

//b4 Extension_flag
//0 No protocol format extension.
//1 Protocol format is extended. Reserved for future use.
#define ISO15693_FL3_Extension_flag						(1<<3)

//b2 RFU 0
//b3 RFU 0
//b5 RFU 0
//b6 RFU 0
//b7 RFU 0
//b8 RFU 0

// Functions
void BP_ISO15693_Inventory(void);
void BP_ISO15693_Init(void);


#endif /* BP_ISO15693_H_ */
