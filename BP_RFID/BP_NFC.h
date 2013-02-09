/*
 * BP_NFC.h
 *
 *  Created on: 09-02-2013
 *      Author: Maciej
 */

#ifndef BP_NFC_H_
#define BP_NFC_H_

#define NFC_RES			0xD5
#define	NFC_ATR			0x01
#define NFC_WUP			0x03
#define NFC_PSL			0x05
#define NFC_DEP 		0x07
#define NFC_DSL			0x09
#define NFC_RLS			0x0B
#define NFC_ACK_HOST 	0x10
#define NFC_NAK_HOST 	0x11

#define	NFC_106AC		0x29
#define NFC_212AC		0x2A
#define NFC_424AC		0x2B
#define	NFC_106PA		0x21
#define NFC_212PA		0x22
#define NFC_424PA		0x23

#define NFC_TAG14443A	0x24
#define NFC_TAG14443B	0x25
#define NFC_TAG15693	0x26
#define TAGNFC_Felica	0x27

#define NFC_TAG106		0x00
#define NFC_TAG212		0x21
#define NFC_TAG424		0x42
#define NFC_TAG848		0x63

#endif /* BP_NFC_H_ */
