/*
 * BP_NDEF.h
 *
 *  Created on: 13-02-2013
 *      Author: Maciej
 */

#ifndef BP_NDEF_H_
#define BP_NDEF_H_

// NFC Forum TS Digital Protocol commands
#define NFC_TS_SENSF_REQ 0x00
#define NFC_TS_SENSF_RES 0x01
#define NFC_TS_ATR_REQ2 0xD5

#define NFC_TS_REQ 0xD4
#define NFC_TS_DEP_REQ 0x06


#define NFC_TS_ATR_REQ2 0xD5

void BP_NDEF_Init(void);

#endif /* BP_NDEF_H_ */
