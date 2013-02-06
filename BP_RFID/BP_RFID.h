/*
 * BP_HAL.h
 *
 *  Created on: Jan 30, 2013
 *      Author: x0184343
 */

#ifndef BP_HAL_H_
#define BP_HAL_H_

/// interface type
///
#define BP_RFID_PARALLEL
//#define BP_RFID_SERIAL
//#define BP_RFID_SERIAL_NOSS

/// led states
///
#define LED_OFF        0
#define LED_ON         1
#define LED_TOGGLE     2
#define LED_BLINK_FAST 3
#define LED_BLINK_SLOW 4

/// Hardware interfacing
// TODO: check if this is needed
#define IS_COMMAND 	0x80
#define DO_READ 	(1<<6)
#define CONTINOUS_MODE (1<<5)

/// PIN DEFINITIONS
///
#define DATA_CLK_PIN GPIO_PIN_0
#define ASK_OOK_PIN  GPIO_PIN_5
#define IRQ_PIN 	 GPIO_PIN_4
#define LED1_PIN 	 GPIO_PIN_5
#define LED2_PIN 	 GPIO_PIN_6
#define EN1_PIN 	 GPIO_PIN_2
#define MOD_PIN 	 GPIO_PIN_2
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

/// TODO: determine minimum size
extern char BP_RFID_BUFFER[];


/// Functions ///
///
// TODO: make inline what should be

void BP_RFID_Init(void);

void BP_RFID_LED1(char);
void BP_RFID_LED2(char);

char BP_RFID_HW_READ_PARALLEL(char);
void BP_RFID_HW_WRITE_PARALLEL(char, char, char);
void BP_RFID_HW_WRITE_PARALLEL_MULTIPLE(char* data, char size);
void BP_RFID_HW_READ_PARALLEL_MULTIPLE(char, char*, char);

void BP_RFID_HW_ENABLE(void);
void BP_RFID_HW_DISABLE(void);

void BP_RFID_LEDS_UPDATE(unsigned long);


void BP_RFID_TRF_Software_Init(void);
void BP_RFID_TRF_Set_ISO(char);



char BP_RFID_Read_Register(char);
void BP_RFID_Read_Registers(char,char*,char);

void BP_RFID_Write_Register(char,char);

#endif /* BP_HAL_H_ */
