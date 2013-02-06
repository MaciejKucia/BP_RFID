/*
 * uart.h
 *
 *  Created on: Jan 30, 2013
 *      Author: x0184343
 */

#ifndef UART_H_
#define UART_H_

void UART_Init(void);
void UARTSend(char *pucBuffer);

void putc(char);
void puts(char*);
void printf(char*, ...);

#endif /* UART_H_ */
