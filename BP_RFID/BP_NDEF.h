/*
 * BP_NDEF.h
 *
 *  Created on: 13-02-2013
 *      Author: Maciej
 */

#ifndef BP_NDEF_H_
#define BP_NDEF_H_

// --- NFC Forum TS Digital Protocol commands ----------------------------------------------
//Technical Specification
//NFC ForumTM
//DIGITAL 1.0
//NFCForum-TS-DigitalProtocol-1.0
//2010-11-17
// 14.5 Command Set

///== Initiator ==

// 14.6 Attribute Request (ATR_REQ)
// The Attribute Request Command (ATR_REQ) is used by the Initiator to activate a Target.
// The Target responds to the ATR_REQ with the Attribute Request Response (ATR_RES).
#define NFC_DEP_ATR_REQ0 0xD4
#define NFC_DEP_ATR_REQ1 0x00

// 14.7 Parameter Selection Request (PSL_REQ)
// The PSL_REQ Command is used to switch communication parameters for the subsequent
// data exchange through the NFC-DEP Protocol.
#define NFC_DEP_PSL_REQ0 0xD4
#define NFC_DEP_PSL_REQ1 0x04

// 14.8 Data Exchange Protocol Request (DEP_REQ)
// The Data Exchange Protocol Request Command (DEP_REQ) is used by the Initiator to exchange
// data with a Target that is configured for the NFC-DEP Protocol. The Target responds to the
// DEP_REQ with the Data Exchange Protocol Request Response (DEP_RES).
#define NFC_DEP_DEP_REQ0 0xD4
#define NFC_DEP_DEP_REQ1 0x06

// 14.9 Deselect Request (DSL_REQ)
// The Deselect Request Command (DSL_REQ) is used by the Initiator to deactivate a Target.
// The Target responds to the DSL_REQ with the Deselect Response (DSL_RES).
#define NFC_DEP_DSL_REQ0 0xD4
#define NFC_DEP_DSL_REQ1 0x08

// 14.10 Release Request (RLS_REQ)
// The Initiator uses the Release Request Command (RLS_REQ) to release a Target, which responds
// to the RLS_REQ with the Release Response (RLS_RES).
#define NFC_DEP_RLS_REQ0 0xD4
#define NFC_DEP_RLS_REQ1 0x0A

//== Target ==
#define NFC_DEP_ATR_RES0 0xD5
#define NFC_DEP_ATR_RES1 0x01

//#define NFC_DEP_PSL_RES 0
//#define NFC_DEP_DEP_RES 0
//#define NFC_DEP_DSL_RES 0
//#define NFC_DEP_RLS_RES 0


//6.6 SENSF_REQ
#define NFC_F_SENSF_REQ




// Logical Link Control (LLC)

#define NFC_LLC_PDU_SYMM	0
#define NFC_LLC_PDU_PAX		1
#define NFC_LLC_PDU_AGF		2
#define NFC_LLC_PDU_UI		3
#define NFC_LLC_PDU_CONNECT	4
#define NFC_LLC_PDU_DISC	5
#define NFC_LLC_PDU_CC		6
#define NFC_LLC_PDU_DM		7
#define NFC_LLC_PDU_FRMR	8
#define NFC_LLC_PDU_SNL		9
#define NFC_LLC_PDU_I		12
#define NFC_LLC_PDU_RR		13
#define NFC_LLC_PDU_RNR		14


void BP_NDEF_Init(void);

#endif /* BP_NDEF_H_ */
