/*
 * TRF797x.h
 *
 *  Created on: Jan 30, 2013
 *      Author: x0184343
 */

#ifndef TRF797X_H_
#define TRF797X_H_

//---- Direct commands ------------------------------------------

#define IDLE						0x00

//This command starts a Power on Reset
#define SOFT_INIT					0x03

//This command executes the initial collision avoidance and sends out IRQ after 5 ms from establishing RF
//field (so the MCU can start sending commands/data). If the external RF field is present (higher than the
//level set in NFC Low Field Detection Level register (0x16)) then the RF field can not be switched on and
//hence a different IRQ is returned.
#define INITIAL_RF_COLLISION		0x04

//This command executes the response collision avoidance and sends out IRQ after 75 �s from establishing
//RF field (so the MCU can start sending commands/data). If the external RF field is present (higher than
//the level set in NFC Low Field Detection Level register (0x16)) then the RF field can not be switched on
//and hence a different IRQ is returned.
#define RESPONSE_RF_COLLISION_N		0x05

//This command executes the response collision avoidance without random delay. It sends out IRQ after 75
//�s from establishing RF field (so the MCU can start sending commands/data). If the external RF field is
//present (higher than the level set in NFC Low Field Detection Level register (0x16)) then the RF field can
//not be switched on and hence a different IRQ is returned.
#define RESPONSE_RF_COLLISION_0		0x06

//The reset command clears the FIFO contents and FIFO status register (0x1C). It also clears the register
//storing the collision error location (0x0E).
#define	RESET						0x0F

//Same as TRANSMIT_CRC with CRC excluded.
#define TRANSMIT_NO_CRC				0x10

//The transmission command must be sent first, followed by transmission length bytes, and FIFO data. The
//reader starts transmitting after the first byte is loaded into the FIFO. The CRC byte is included in the
//transmitted sequence.
#define TRANSMIT_CRC				0x11

//Same as DELAY_TRANSMIT_CRC with CRC excluded.
#define DELAY_TRANSMIT_NO_CRC		0x12

//The transmission command must be sent first, followed by the transmission length bytes, and FIFO data.
//The reader transmission is triggered by the TX timer.
#define DELAY_TRANSMIT_CRC			0x13

//When this command is received, the reader transmits the next slot command. The next slot sign is defined
//by the protocol selection.
#define TRANSMIT_NEXT_SLOT			0x14

//TODO:
#define CLOSE_SLOT_SEQUENCE			0x15

//The block receiver command puts the digital part of receiver (bit decoder and framer) in reset mode. This
//is useful in an extremely noisy environment, where the noise level could otherwise cause a constant
//switching of the subcarrier input of the digital part of the receiver. The receiver (if not in reset) would try to
//catch a SOF signal, and if the noise pattern matched the SOF pattern, an interrupt would be generated,
//falsely signaling the start of an RX operation. A constant flow of interrupt requests can be a problem for
//the external system (MCU), so the external system can stop this by putting the receive decoders in reset
//mode. The reset mode can be terminated in two ways. The external system can send the enable receiver
//command. The reset mode is also automatically terminated at the end of a TX operation. The receiver can
//stay in reset after end of TX if the RX wait time register (0x08) is set. In this case, the receiver is enabled
//at the end of the wait time following the transmit operation.
#define STOP_DECODERS				0x16

//This command clears the reset mode in the digital part of the receiver if the reset mode was entered by
//the block receiver command.
#define RUN_DECODERS				0x17

//The level of the RF carrier at RF_IN1 and RF_IN2 inputs is measured. Operating range between 300 mVP
//and 2.1 VP (step size is 300 mV). The two values are displayed in the RSSI levels register (0x0F). The
//command is intended for diagnostic purposes to set correct RF_IN levels. Optimum RFIN input level is
//approximately 1.6 VP or code 5 to 6. The nominal relationship between the RF peak level and RSSI code
//is shown in Table 5-20 and in Section 5.4.1.1.
//NOTE:
//If the command is executed immediately after power-up and before any communication with
//a tag is performed, the command must be preceded by Enable RX command. The Check RF
//commands require full operation, so the receiver must be activated by Enable RX or by a
//normal Tag communication for the Check RF command to work properly
#define CHECK_INTERNAL_RF			0x18
//RF_IN1 [mV_PP] 300 600 900 1200 1500 1800 2100


//This command can be used in active mode when the RF receiver is switched on but RF output is switched
//off. This means bit B1 = 1 in Chip Status Control Register. The level of RF signal received on the antenna
//is measured and displayed in the RSSI Levels register (0x0F). The relation between the 3 bit code and the
//external RF field strength [A/m] must be determinate by calculation or by experiments for each antenna
//type as the antenna Q and connection to the RF input influence the result.
//NOTE:
//If the command is executed immediately after power-up and before any communication with
//a tag is performed, the command must be preceded by an Enable RX command. The Check
//RF commands require full operation, so the receiver must be activated by Enable RX or by a
//normal Tag communication for the Check RF command to work properly.
#define CHECK_EXTERNAL_RF			0x19
//RF_IN1 [mV_PP] 40 60 80 100 140 180 300

//This command should be executed when the MCU determines that no TAG response is coming and when
//the RF and receivers are switched ON. When this command is received, the reader observes the digitized
//receiver output. If more than two edges are observed in 100 ms, the window comparator voltage is
//increased. The procedure is repeated until the number of edges (changes of logical state) of the digitized
//reception signal is less than 2 (in 100 ms). The command can reduce the input sensitivity in 5-dB
//increments up to 15 dB. This command ensures better operation in a noisy environment. The gain setting
//is reset to maximum gain at EN = 0 and POR = 1.
#define ADJUST_GAIN					0x1A

//---- END -------------------------------------------------------

//---- Reader registers ------------------------------------------

// [W] - write only
// [R] - read only

#define TRF_REG_CHIP_STATE_CONTROL			0x00 // Chip Status Control
#define TRF_REG_ISO_CONTROL					0x01 // ISO Control

#define TRF_REG_ISO_14443B_OPTIONS			0x02 // ISO14443B TX options
#define TRF_REG_ISO_14443A_OPTIONS			0x03 // ISO14443A high bit rate options
#define TRF_REG_TX_TIMER_EPC_HIGH			0x04 // TX timer setting, H-byte
#define TRF_REG_TX_TIMER_EPC_LOW			0x05 // TX timer setting, L-byte
#define TRF_REG_TX_PULSE_LENGTH_CONTROL		0x06 // TX pulse-length control
#define TRF_REG_RX_NO_RESPONSE_WAIT_TIME	0x07 // RX no response wait time
#define TRF_REG_RX_WAIT_TIME				0x08 // RX wait time
#define TRF_REG_MODULATOR_CONTROL			0x09 // Modulator and SYS_CLK control
#define TRF_REG_RX_SPECIAL_SETTINGS			0x0A // RX Special Setting
#define TRF_REG_REGULATOR_CONTROL			0x0B // Regulator and I/O control

#define TRF_REG_SPECIAL_1					0x10 // Special Function Register, Preset 0x00
#define TRF_REG_SPECIAL_2					0x11 // Special Function Register, Preset 0x00

#define TRF_REG_FIFO_Levels					0x14 // Adjustable FIFO IRQ Levels Register
//#define TRF_REG_RESERVED					0x15 // Reserved R/W

#define TRF_REG_NFC_Low_Field				0x16 // NFC Low Field Detection Level
#define TRF_REG_NFCID1_Number				0x17 // NFCID1 Number (up to 10 bytes wide) [W]
#define TRF_REG_NFC_Target_Det_Lvl			0x18 // NFC Target Detection Level
#define TRF_REG_NFC_Target_Protocol			0x19 // NFC Target Protocol

#define TRF_REG_IRQ_STATUS					0x0C // IRQ Status Register
#define TRF_REG_IRQ_MASK					0x0D // Collision Position and Interrupt Mask Register
#define	TRF_REG_COLLISION_POSITION			0x0E // Collision position [R]
#define TRF_REG_RSSI_LEVELS					0x0F // RSSI levels and oscillator status [R]

#define TRF_REG_RAM_1						0x12 //
#define TRF_REG_RAM_2						0x13 // RAM

#define TRF_REG_TEST_SETTINGS_1				0x1A //
#define TRF_REG_TEST_SETTINGS_2				0x1B // Test Register

#define TRF_REG_FIFO_CONTROL				0x1C // FIFO status
#define TRF_REG_TX_LENGTH_BYTE_1			0x1D // Tx length byte 1
#define TRF_REG_TX_LENGTH_BYTE_2			0x1E // Tx length byte 2
#define TRF_REG_FIFO						0x1F // FIFO I/O register

//---- END -------------------------------------------------------

//---- IRQ -------------------------------------------------------

/// IRQ_BITS TODO

#define TRF_IRQ_NORESP	(1<<0) //Trigger for MCU to send next EOF/Slot Marker as defined by No Response Wait Time Register (0x07) (for ISO15693)
#define TRF_IRQ_COL		(1<<1) //
#define TRF_IRQ_ERR3	(1<<2)
#define TRF_IRQ_ERR2	(1<<3)
#define TRF_IRQ_ERR1	(1<<4)
#define TRF_IRQ_FIFO	(1<<5)
#define TRF_IRQ_RX		(1<<6)
#define TRF_IRQ_TX		(1<<7)

// Table 5-17. IRQ and Status Register (0x0C) for NFC and Card Emulation Operation

#define TRF_IRQ_NFC_Tx_End							(1<<7)
#define TRF_IRQ_NFC_Rx_Start						(1<<6)
#define TRF_IRQ_NFC_FIFO_High						(1<<5)
#define TRF_IRQ_NFC_Protocol_Error					(1<<4)
#define TRF_IRQ_NFC_SDD_Finished					(1<<3) //8
#define TRF_IRQ_NFC_RF_Field_Change					(1<<2) //4
#define TRF_IRQ_NFC_Col_Avoid_Finished				(1<<1)
#define TRF_IRQ_NFC_Col_Avoid_Failed				(1<<0)

//---- END -------------------------------------------------------

//---- REGISTER SETTINGS -----------------------------------------

// Table 6-2. Chip Status Control Register (0x00)
//
//

//0 = Active Mode -> Active Mode (default)
//1 = Standby Mode -> Standby mode keeps all supply regulators, 13.56-MHz SYS_CLK oscillator running. (typical start-up time to full operation 100 �s)
#define TRF_CSR_STANDBY			(1<<7)

//1 = Direct Mode 0 or 1 -> Provides user direct access to AFE (Direct Mode 0) or allows user to add their own framing (Direct Mode 1). Bit 6 of ISO Control register must be set by user before entering Direct Mode 0 or 1.
//0 = Direct Mode 2 (default) -> Uses SPI or parallel communication with automatic framing and ISO decoders
#define TRF_CSR_DIRECT			(1<<6)

//1 = RF output active      Transmitter on, receivers on
//0 = RF output not active  Transmitter off
#define TRF_CSR_RF_ON 			(1<<5)

//1 = half output power TX_OUT (pin 5) = 4-ohm output impedance P = 33 mW (+15 dBm) at 3.3 V
//0 = full output power TX_OUT (pin 5) = 4-ohm output impedance P = 70 mW (+18 dBm) at 3.3 V
#define TRF_CSR_HALFPOWER		(1<<4)

//1 = selects Aux  RX input RX_IN2 input is used
//0 = selects Main RX input RX_IN1 input is used
#define TRF_CSR_RX_AUX 			(1<<3)

//1 = AGC on  Enables AGC (AGC gain can be set in register 0x0A)
//0 = AGC off AGC block is disabled
#define TRF_CSR_AGC_ON 			(1<<2)

//1 = Receiver activated for external field measurement. Forced enabling of receiver and TX oscillator. Used for external field measurement
//0 = Automatic Enable Allows enable of the receiver by Bit 5 of this register (0x00)
#define TRF_CSR_RECEIVER_ON 	(1<<1)

//Selects the VIN voltage range
//1 = 5 V operation
//0 = 3 V operation
#define TRF_CSR_5V 				(1<<0)



// Table 6-3. ISO Control Register (0x01)
//
//

//0 0 0 0 0 ISO15693 low bit rate,  6.62 kbps,  one subcarrier,    1 out of 4
//0 0 0 0 1 ISO15693 low bit rate,  6.62 kbps,  one subcarrier,    1 out of 256
//0 0 0 1 0 ISO15693 high bit rate, 26.48 kbps, one subcarrier,    1 out of 4 Default for reader
//0 0 0 1 1 ISO15693 high bit rate, 26.48 kbps, one subcarrier,    1 out of 256
//0 0 1 0 0 ISO15693 low bit rate,  6.67 kbps,  double subcarrier, 1 out of 4
//0 0 1 0 1 ISO15693 low bit rate,  6.67 kbps,  double subcarrier, 1 out of 256
//0 0 1 1 0 ISO15693 high bit rate, 26.69 kbps, double subcarrier, 1 out of 4
//0 0 1 1 1 ISO15693 high bit rate, 26.69 kbps, double subcarrier, 1 out of 256
#define TRF_PROTOCOL_ISO15693 0
#define TRF_PROTOCOL_ISO15693_Double_SubCarrier (1<<2)
#define TRF_PROTOCOL_ISO15693_High_Bit_Rate 	(1<<1)
#define TRF_PROTOCOL_ISO15693_1_out_of_256  	(1<<0)

//0 1 0 0 0 ISO14443A RX      bit rate, 106 kbps RX bit rate
//0 1 0 0 1 ISO14443A RX high bit rate, 212 kbps
//0 1 0 1 0 ISO14443A RX high bit rate, 424 kbps
//0 1 0 1 1 ISO14443A RX high bit rate, 848 kbps
#define TRF_PROTOCOL_ISO14443A 					(1<<3)
#define TRF_PROTOCOL_ISO14443A_212				(1<<0)
#define TRF_PROTOCOL_ISO14443A_424				(1<<1)
#define TRF_PROTOCOL_ISO14443A_848				(1<<0)|(1<<1)

//0 1 1 0 0 ISO14443B RX      bit rate, 106 kbps RX bit rate
//0 1 1 0 1 ISO14443B RX high bit rate, 212 kbps
//0 1 1 1 0 ISO14443B RX high bit rate, 424 kbps
//0 1 1 1 1 ISO14443B RX high bit rate, 848 kbps
#define TRF_PROTOCOL_ISO14443B 					(1<<3)|(1<<2)
#define TRF_PROTOCOL_ISO14443B_212				(1<<0)
#define TRF_PROTOCOL_ISO14443B_424				(1<<1)
#define TRF_PROTOCOL_ISO14443B_848				(1<<0)|(1<<1)

//1 1 0 1 0 FeliCa 212 kbps
//1 1 0 1 1 FeliCa 424 kbps
#define TRF_PROTOCOL_FeliCa 		(1<<3)|(1<<4)|(1<<1)
#define TRF_PROTOCOL_FeliCa_424 	(1<<0)

//B7 rx_crc_n CRC Receive selection
//0 = RX CRC (CRC is present in the response)
//1 = no RX CRC (CRC is not present in the response)
#define TRF_PROTOCOL_CRC			(1<<7)

//B6 dir_mode Direct mode type selection
//0 = Direct Mode 0
//1 = Direct Mode 1
#define TRF_PROTOCOL_Dir_Mode		(1<<6)

//B5 rfid RFID / Reserved
//0 = RFID Mode
//1 = NFC or Card Emulation Mode
#define TRF_PROTOCOL_RFID			(1<<5)

//ISO Control Register ISO_x Settings,
//NFC Mode (B5 = 1, B2 = 0) or Card Emulation (B5 = 1, B2 = 1)
//    NFC        CARD EMU
//0 0 N/A        ISO14443A
//0 1 106 kbps   ISO14443B
//1 0 212 kbps   N/A
//1 1 424 kbps   N/A
#define  TRF_PROTOCOL_NFC_MODE_106kbps 			1|TRF_PROTOCOL_RFID
#define  TRF_PROTOCOL_NFC_MODE_212kbps 			2|TRF_PROTOCOL_RFID
#define  TRF_PROTOCOL_NFC_MODE_424kbps 			3|TRF_PROTOCOL_RFID

#define  TRF_PROTOCOL_NFC_MODE_EMU_ISO14443A 	0|TRF_PROTOCOL_RFID|(1<<2)
#define  TRF_PROTOCOL_NFC_MODE_EMU_ISO14443B 	1|TRF_PROTOCOL_RFID|(1<<2)

// Table 6-6. ISO14443B TX Options Register (0x02)
//
//

// Three bit code defines the number of etu (0-7) which separate two characters. ISO14443B TX only
//
// TX EGT time select MSB TODO:??
#define TRF_ISO14443B_TX_Time2 (1<<7)
// TX EGT time select
#define TRF_ISO14443B_TX_Time1 (1<<6)
// TX EGT time select LSB
#define TRF_ISO14443B_TX_Time0 (1<<5)

// ISO14443B TX only
//
//1 = EOF-> 0 length 11 etu
//0 = EOF-> 0 length 10 etu
#define TRF_ISO14443B_TX_EOF0_L11 (1<<4)
//1 = SOF-> 1 length 03 etu
//0 = SOF-> 1 length 02 etu
#define TRF_ISO14443B_TX_SOF1_L03 (1<<3)
//1 = SOF-> 0 length 11 etu
//0 = SOF-> 0 length 10 etu
#define TRF_ISO14443B_TX_SOF0_L11 (1<<2)
//1 = EGT after each byte
//0 = EGT after last byte is omitted
#define TRF_ISO14443B_TX_EGT (1<<1)

// For use with Auto SDD configuration, makes B6 in ISO14443A response 1 or 0, indicating Layer 4 compliance (or not), for all other cases, this bit is unused
// Auto SDD_SAK
// 1 = ISO14443A Layer 4 compliant (in SAK response)
// 0 = Not Layer 4 compliant (in SAK response)
#define TRF_ISO14443B_TX_Auto_SDD_SAK (1<<0)



// Table 6-14. RX Special Setting Register (Address 0x0A)
//
//

//B7 C212 Bandpass 110 kHz to 570 kHz Appropriate for 212-kHz subcarrier system (FeliCa)
#define TRF_NFC_RX_SPEC_FeliCa 				(1<<7)

//B6 C424 Bandpass 200 kHz to 900 kHz Appropriate for 424-kHz subcarrier used in ISO15693
#define TRF_NFC_RX_SPEC_ISO15693			(1<<6)

//Appropriate for Manchester-coded 848-kHz subcarrier used in ISO14443A
#define TRF_NFC_RX_SPEC_ISO14443			(1<<5)

//B5 M848 Bandpass 450 kHz to 1.5 MHz Gain reduced for 18 dB  Appropriate for highest bit rate (848 kbps) used in high-bit-rate ISO14443
#define TRF_NFC_RX_SPEC_848kbps_ISO14443	(1<<4)

// AGC activation level changed from five times the digitizing level to three times the digitizing level.
// AGC activation level change:
//  1 = 3x
//  0 = 5x
#define TRF_NFC_RX_SPEC_AGC3x				(1<<1)

// AGC action can be done any time during receive process. It is not limited to the start of receive ("max hold").
// AGC action is not limited in time:
//  1 = continuously � no time limit
//  0 = 8 subcarrier pulses
#define TRF_NFC_RX_SPEC_AGC_Action			(1<<0)

#define TRF_NFC_RX_SPEC_Gain_Reduction_0dB	(0<<2) //Gain reduction
#define TRF_NFC_RX_SPEC_Gain_Reduction_5dB  (1<<2) //
#define TRF_NFC_RX_SPEC_Gain_Reduction_10dB (2<<2) //
#define TRF_NFC_RX_SPEC_Gain_Reduction_15dB (3<<2) //Sets the RX gain reduction, and reduces sensitivity

// Table 6-28. NFC Low Field Level Register (0x16)
//
//

// NFC passive 106-kbps and ISO14443A card emulation
#define TRF_NFC_Field_Detection_Disable_Clock_Extractor (1<<7)

#define TRF_NFC_Field_Detection_Level_Not_Active 	0
#define TRF_NFC_Field_Detection_Level_480mV			1
#define TRF_NFC_Field_Detection_Level_350mV			2
#define TRF_NFC_Field_Detection_Level_250mV			3
#define TRF_NFC_Field_Detection_Level_220mV			4
#define TRF_NFC_Field_Detection_Level_190mV			5
#define TRF_NFC_Field_Detection_Level_180mV			6
#define TRF_NFC_Field_Detection_Level_170mW			7



// Table 5-14. NFC Target Detection Level Register (0x18)
//
//


#define TRF_NFC_Target_Detection_Level_Not_Active 	0
#define TRF_NFC_Target_Detection_Level_480mV		1
#define TRF_NFC_Target_Detection_Level_350mV		2
#define TRF_NFC_Target_Detection_Level_250mV		3
#define TRF_NFC_Target_Detection_Level_220mV		4
#define TRF_NFC_Target_Detection_Level_190mV		5
#define TRF_NFC_Target_Detection_Level_180mV		6
#define TRF_NFC_Target_Detection_Level_170mW		7

// Extended Range
#define TRF_NFC_Target_Detection_Level_1500mV		1
#define TRF_NFC_Target_Detection_Level_700mV		2
#define TRF_NFC_Target_Detection_Level_500mV		3
#define TRF_NFC_Target_Detection_Level_450mV		4
#define TRF_NFC_Target_Detection_Level_400mV		5
#define TRF_NFC_Target_Detection_Level_320mV		6
#define TRF_NFC_Target_Detection_Level_280mW		7

#define TRF_NFC_Target_Detection_Extended_Range 	(1<<3)

#define TRF_NFC_Target_Detection_SDD_Enabled		(1<<5)

#define TRF_NFC_Target_Detection_NFCID1_4			0
#define TRF_NFC_Target_Detection_NFCID1_7			(1<<6)
#define TRF_NFC_Target_Detection_NFCID1_10			(1<<7)


// Table 5-16. NFC Target Protocol Register (0x19)
#define TRF_NFC_Target_Protocol_RF_Level_wake   	(1<<7) //80
#define TRF_NFC_Target_Protocol_RF_Level_collision	(1<<6) //40
// bit 5 n/a
#define TRF_NFC_Target_Protocol_FeliCa				(1<<4) //10
#define TRF_NFC_Target_Protocol_PassiveOrTag		(1<<3) //08

#define TRF_NFC_Target_Protocol_ISO14443B			(1<<2) //04

//Bit rate of first received command
#define TRF_NFC_Target_Protocol_106kbps				1	  //01
#define TRF_NFC_Target_Protocol_212kbps				2     //02
#define TRF_NFC_Target_Protocol_424kbps				3     //03




// Power Supply Regulator Setting
// VDD_RF = 3.3 V, VDD_A = 3.3 V, VDD_X = 3.3 V
// TODO: check if needed
#define POWER_SETTING 0x06


/// Modulator and SYS_CLK Control Register 0x09
/// TODO:
///
#define MOD_ASK10	0
#define MOD_OOK100  1
#define MOD_ASK7	2
#define MOD_ASK8_5  3
#define MOD_ASK13 	4
#define MOD_ASK16 	5
#define MOD_ASK22 	6
#define MOD_ASK30 	7

#define MOD_ASKOOK_ENABLE 8

//#define MOD_SYS_CLK (1<<4) (1<<5)

#define MOD_ASKOOK_EXTERNAL (1<<6)

//#define MOD_27MHZ (1<<7) this is not supported by booster pack hardware


#endif /* TRF797X_H_ */
