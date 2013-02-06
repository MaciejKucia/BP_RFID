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

//This command executes the response collision avoidance and sends out IRQ after 75 µs from establishing
//RF field (so the MCU can start sending commands/data). If the external RF field is present (higher than
//the level set in NFC Low Field Detection Level register (0x16)) then the RF field can not be switched on
//and hence a different IRQ is returned.
#define RESPONSE_RF_COLLISION_N		0x05

//This command executes the response collision avoidance without random delay. It sends out IRQ after 75
//µs from establishing RF field (so the MCU can start sending commands/data). If the external RF field is
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
//RF_IN1 [mVPP] 300 600 900 1200 1500 1800 2100

#define CHECK_EXTERNAL_RF			0x19
#define ADJUST_GAIN					0x1A

//---- Reader registers ------------------------------------------

#define CHIP_STATE_CONTROL			0x00
#define ISO_CONTROL					0x01
#define ISO_14443B_OPTIONS			0x02
#define ISO_14443A_OPTIONS			0x03
#define TX_TIMER_EPC_HIGH			0x04
#define TX_TIMER_EPC_LOW			0x05
#define TX_PULSE_LENGTH_CONTROL		0x06
#define RX_NO_RESPONSE_WAIT_TIME	0x07
#define RX_WAIT_TIME				0x08
#define MODULATOR_CONTROL			0x09
#define RX_SPECIAL_SETTINGS			0x0A
#define REGULATOR_CONTROL			0x0B
#define IRQ_STATUS					0x0C	// IRQ Status Register
#define IRQ_MASK					0x0D	// Collision Position and Interrupt Mask Register
#define	COLLISION_POSITION			0x0E
#define RSSI_LEVELS					0x0F
#define SPECIAL_FUNCTION			0x10
#define RAM_START_ADDRESS			0x11	//RAM is 6 bytes long (0x11 - 0x16)
#define NFC_LOW_DETECTION			0x16
#define NFCID						0x17
#define NFC_TARGET_LEVEL			0x18
#define NFC_TARGET_PROTOCOL			0x19
#define TEST_SETTINGS_1				0x1A
#define TEST_SETTINGS_2				0x1B
#define FIFO_CONTROL				0x1C
#define TX_LENGTH_BYTE_1			0x1D
#define TX_LENGTH_BYTE_2			0x1E
#define FIFO						0x1F

/// IRQ_BITS

#define IRQ_NORESP	(1<<0) //Trigger for MCU to send next EOF/Slot Marker as defined by No Response Wait Time Register (0x07) (for ISO15693)
#define IRQ_COL		(1<<1) //
#define IRQ_ERR3	(1<<2)
#define IRQ_ERR2	(1<<3)
#define IRQ_ERR1	(1<<4)
#define IRQ_FIFO	(1<<5)
#define IRQ_RX		(1<<6)
#define IRQ_TX		(1<<7)


// TODO: finish this
//0 0 0 0 0 ISO15693 low bit rate, 6.62 kbps, one subcarrier, 1 out of 4
#define ISO_PROTOCOL_0 0
//0 0 0 0 1 ISO15693 low bit rate, 6.62 kbps, one subcarrier, 1 out of 256
#define ISO_PROTOCOL_1 1
//0 0 0 1 0 ISO15693 high bit rate, 26.48 kbps, one subcarrier, 1 out of 4 Default for reader
#define ISO_PROTOCOL_2 2
//0 0 0 1 1 ISO15693 high bit rate, 26.48 kbps, one subcarrier, 1 out of 256
#define ISO_PROTOCOL_3 3
//0 0 1 0 0 ISO15693 low bit rate, 6.67 kbps, double subcarrier, 1 out of 4
#define ISO_PROTOCOL_4 4
//0 0 1 0 1 ISO15693 low bit rate, 6.67 kbps, double subcarrier, 1 out of 256
#define ISO_PROTOCOL_5 5
//0 0 1 1 0 ISO15693 high bit rate, 26.69 kbps, double subcarrier, 1 out of 4
#define ISO_PROTOCOL_6 6
//0 0 1 1 1 ISO15693 high bit rate, 26.69 kbps, double subcarrier,1 out of 256
//0 1 0 0 0 ISO14443A RX bit rate, 106 kbps RX bit rate
//0 1 0 0 1 ISO14443A RX high bit rate, 212 kbps
//0 1 0 1 0 ISO14443A RX high bit rate, 424 kbps
//0 1 0 1 1 ISO14443A RX high bit rate, 848 kbps
//0 1 1 0 0 ISO14443B RX bit rate, 106 kbps RX bit rate
//0 1 1 0 1 ISO14443B RX high bit rate, 212 kbps
//0 1 1 1 0 ISO14443B RX high bit rate, 424 kbps
//0 1 1 1 1 ISO14443B RX high bit rate, 848 kbps
//1 1 0 1 0 FeliCa 212 kbps
//1 1 0 1 1 FeliCa 424 kbps

// Power Supply Regulator Setting
// VDD_RF = 3.3 V, VDD_A = 3.3 V, VDD_X = 3.3 V
// TODO: check
#define POWER_SETTING 0x06

/// Chip Status Control Register (0x00)

#define CSR_5V 				(1<<0)
#define CSR_RECEIVER_ON 	(1<<1)
#define CSR_AGC_ON 			(1<<2)
#define CSR_RX_AUX 			(1<<3) //TODO: ??
#define CSR_HALFPOWER		(1<<4)
#define CSR_RF_ON 			(1<<5)
#define CSR_DIRECT0v1		(1<<6)
#define CSR_STANDBY			(1<<7)

/// Modulator and SYS_CLK Control Register 0x09

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
