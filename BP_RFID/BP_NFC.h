/*
 * BP_NFC.h
 *
 *  Created on: 09-02-2013
 *      Author: Maciej
 */

#ifndef BP_NFC_H_
#define BP_NFC_H_

#define RES			0xD5
#define	ATR			0x01
#define WUP			0x03
#define PSL			0x05
#define DEP 		0x07
#define DSL			0x09
#define RLS			0x0B
#define ACK_HOST 	0x10
#define NAK_HOST 	0x11

#define	NFC106AC	0x29
#define NFC212AC	0x2A
#define NFC424AC	0x2B
#define	NFC106PA	0x21
#define NFC212PA	0x22
#define NFC424PA	0x23

#define TAG14443A	0x24
#define TAG14443B	0x25
#define TAG15693	0x26
#define TAGFelica	0x27

#define TAG106		0x00
#define TAG212		0x21
#define TAG424		0x42
#define TAG848		0x63

#endif /* BP_NFC_H_ */
