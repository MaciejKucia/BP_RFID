/*
 * BP_HAL.h
 *
 *  Created on: Jan 30, 2013
 *      Author: x0184343
 */

#ifndef BP_HAL_H_
#define BP_HAL_H_

extern unsigned volatile int 	SysTickCounter;

/// interface type
///
#define BP_RFID_PARALLEL

/// Serial SPI with SS
//#define BP_RFID_SERIAL



/// led states
///
#define LED_OFF        0
#define LED_ON         1
#define LED_TOGGLE     2
#define LED_BLINK_FAST 3
#define LED_BLINK_SLOW 4

/// PIN DEFINITIONS
///
#define DATA_CLK_PIN GPIO_PIN_0
#define ASK_OOK_PIN  GPIO_PIN_5
#define IRQ_PIN 	 GPIO_PIN_4
#define LED1_PIN 	 GPIO_PIN_5
#define LED2_PIN 	 GPIO_PIN_6
#define MOD_PIN 	 GPIO_PIN_2
#define EN1_PIN 	 GPIO_PIN_2
#define T1_PIN 		 GPIO_PIN_3
#define T2_PIN 		 GPIO_PIN_3
#define T3_PIN 		 GPIO_PIN_4
#define T4_PIN 		 GPIO_PIN_5
#define T5_PIN 		 GPIO_PIN_6
#define T6_PIN 		 GPIO_PIN_7
#define T7_PIN 		 GPIO_PIN_6
#define T8_PIN 		 GPIO_PIN_7

/// Variables ///
///

extern char BP_RFID_RX_BUFFER[];
extern volatile char BP_RFID_RX_BUFFER_COUNT;


/// last IRQ flags
extern volatile char TRF_IRQ_LAST_FLAGS;

/// IRQ happened flags
extern volatile char TRF_IRQ_SEMAPHORE;

// --- Functions ----------------------------------------------------------------------------

// Special

void BP_RFID_Init(void);
void BP_RFID_Set_IRQ_Callback(void (*function)(char));
void BP_RFID_Tick();

// HardWare interfacing
//
void BP_RFID_HW_PARALLEL_INIT(void);

void BP_RFID_HW_LEDS_UPDATE(unsigned long);
void BP_RFID_HW_LED1(char);
void BP_RFID_HW_LED2(char);

void BP_RFID_HW_DISABLE(void);
void BP_RFID_HW_ENABLE(void);

void BP_RFID_HW_INT_DISABLE(void);
void BP_RFID_HW_INT_ENABLE(void);

void BP_RFID_HW_PARALLEL_START(void);
void BP_RFID_HW_PARALLEL_STOP(void);
void BP_RFID_HW_PARALLEL_STOP_MULTIPLE(void);

void BP_RFID_HW_PARALLEL_SIMPLE_WRITE(char);

char BP_RFID_HW_PARALLEL_READ(char);
void BP_RFID_HW_PARALLEL_WRITE(char, char, char);

void BP_RFID_HW_PARALLEL_READ_MULTIPLE(char, char*, char);
void BP_RFID_HW_PARALLEL_WRITE_MULTIPLE(char* data, char size);


// Hardware independent
// Chip support
//

char BP_RFID_TRF_Transmit(char*, unsigned short);
void BP_RFID_TRF_Read_Registers(char,char*,char);
void BP_RFID_TRF_Write_Register(char,char);
char BP_RFID_TRF_FIFO_How_Many_Bytes(void);
void BP_RFID_TRF_Modulator_Control(char);
void BP_RFID_TRF_Set_NFCID(char, char*);
void BP_RFID_TRF_Reset_Decoders(void);
char BP_RFID_TRF_Read_Register(char);
void BP_RFID_TRF_Stop_Decoders(void);
void BP_RFID_TRF_Run_Decoders(void);
void BP_RFID_TRF_Turn_RF_Off(void);
void BP_RFID_TRF_FIFO_Reset(void);
void BP_RFID_TRF_Turn_RF_On(void);
char BP_RFID_TRF_IRQ_Clear(void);
char BP_RFID_TRF_Get_RSSI(void);
void BP_RFID_TRF_Set_ISO(char);

char BP_RFID_TRF_Wait_For_Rx_End(void);
char BP_RFID_TRF_Wait_For_Tx_End(void);


#endif /* BP_HAL_H_ */
