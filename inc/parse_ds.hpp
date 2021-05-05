#pragma once
/* parse_ds.hpp: Davicom RFID packet data structures (parsing result). */
/*                                                                              */
/*==============================================================================*/
/* Header Name  : parse_ds.h */
/* Purpose      : RFID communication */
/* Autor        : Richard Chuang */
/* Email        : richard_chung@davicom.com.tw */
/* Description  : */
/* Reference    : */
/* Copyright    : (c) Davicom Semiconductor Inc. All Rights Reserved. */
/* -----------------------------------------------------------------------------*/
/* Version      : 1.0.0.0 */
/* Update       : 2020-11-02 */
/* Modified     : Richard Chung */
/* Description  : */
/* Version      : 1.1.0.0 */
/* Update       : 2021-02-26 */
/* Modified     : Tom Sun */
/* Description  : */
/* -----------------------------------------------------------------------------*/
/* How to use: */
/*                                                                              */


#include "TString.h"
#define _T(x) x

//==============================================================================
//--- Declarative Identifier  ---
//==============================================================================
//------------------------------------------------------------------------------
//--- ID Field Definition ---
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Reader
//------------------------------------------------------------------------------
#define CMD_RFID_READER_VERSION 'V' // display reader firmware version
#define CMD_RFID_READER_ID 'S'      // display reader ID
#define CMD_RFID_READ_BANK 'R'      // Read tag memory data
//#define CMD_RFID_READ_TAG					'R'
//// Read tag memory data
#define CMD_RFID_READ_EPC 'Q' // Display tag EPC ID
#define CMD_RFID_READ_TID 'R'
//#define CMD_RFID_WRITE_TAG					'W'
//// Write data to tag memory
#define CMD_RFID_WRITE_BANK 'W'     // Write data to tag memory
#define CMD_RFID_READ_USER_DATA 'R' //
#define CMD_RFID_READ_EPC_WITH_TID 'Q'
#define CMD_RFID_READ_SINGLE_EPC 'Q' // Display tag EPC ID/Read Single-Tag EPC
#define CMD_RFID_READ_MULTI_EPC 'U'  // Read Multi-Tag EPC

#define CMD_RFID_SELECT_MATCHING 'T' // Select matching tag
#define CMD_RFID_SET_SESSION 'T'     // Session 0~3
#define CMD_RFID_PASSWORD    'P'
#define CMD_RFID_READ_DATE _T("@SETDATE")
//------------------------------------------------------------------------------
// Reader Error Code
//------------------------------------------------------------------------------
#define ERROR_CODE_OTHER _T('0')              // 0 : other error
#define ERROR_CODE_MEMORY_OVERRUN _T('3')     // 3 : memory overrun
#define ERROR_CODE_MEMORY_LOCKED _T('4')      // 4 : memory locked
#define ERROR_CODE_INSUFFICIENT_POWER _T('B') // B : Insufficient power
#define ERROR_CODE_NON_SPECIFIC _T('F')       // F : Non - specific error

// W error
// Z00~Z1F: words write
// 3Z00~3Z1F: error code and words write
#define ERROR_STRING_ZXX _T("Z")   // Z : Z00~Z1F: words write
#define ERROR_STRING_3ZXX _T("3Z") // 3Z : error code and words write
#define ERROR_STRING_OK _T("OK")   // OK :
#define ERROR_STRING_END _T("END") // @END : End of packet

//------------------------------------------------------------------------------
// Reader
//------------------------------------------------------------------------------
#define MAX_MEMORY_BANK_ADDRESS 0x3FFF
#define MAX_MEMORY_BANK_LENGTH 0x1E
// Response EPC Content: PC(4 Bytes) + EPC + CRC16(4 Bytes)
#define LEN_EPC_PEOTOCOL 4     // Protocol Control
#define LEN_EPC_CRC16 4        // CRC16
#define LEN_EPC_OFFSET_LEFT 4  // Trim the left side of the string
#define LEN_EPC_OFFSET_RIGHT 4 // Trim the right side of the string

//------------------------------------------------------------------------------
// Reader Interface Error Code
//------------------------------------------------------------------------------
#define ERROR_RFID_SUCCESS 0x00000000  // The operation completed successfully.
#define ERROR_RFID_NO_ERROR 0x00000000 // The operation completed successfully.

#define ERROR_RFID_MEMORY_OVERRUN 0x00000003     // memory overrun
#define ERROR_RFID_MEMORY_LOCKED 0x00000004      // memory locked
#define ERROR_RFID_INSUFFICIENT_POWER 0x0000000B // Insufficient power
#define ERROR_RFID_UNKNOW 0x000000FE             // Other error
#define ERROR_RFID_NON_SPECIFIC 0x000000FF       // Non-specific error

//------------------------------------------------------------------------------
// Packet Buffer Size
//------------------------------------------------------------------------------
#define MAX_SEND_BUFFER 1536
#define MAX_RECV_BUFFER 1536
#define MAX_PARSE_BUFFER 1024

#define TEXT_RFID_ANTENNA _T("Antenna")

//==============================================================================
//--- Declarative Structure ---
//==============================================================================
// RFID: Radio Frequency Identification
// BankType:
//     AccessPwd
//     KillPwd
//     EPC
//     TID
//     USER

typedef enum _RFID_BANK_TYPE_ {
	TP_RB_ACCESS_PWD, // AccessPwd
	TP_RB_KILL_PWD,   // KillPwd
	TP_RB_EPC,        // EPC (Electronic Product Code)
	TP_RB_TID,        // TID (Tag Identifier)
	TP_RB_USER        // User Data
} RFID_BANK_TYPE,
	*PRFID_BANK_TYPE;

// LockType:
//     Lock
//     PermaLock
//     UnLock
typedef enum _RFID_LOCK_TYPE_ {
	TP_RL_LOCK,          // Lock
	TP_RL_PERMAENT_LOCK, // Permanent lock
	TP_RL_UNLOCK,        // UnLock

} RFID_LOCK_TYPE,
	*PRFID_LOCK_TYPE;

// Regulation/ Regulations
// US US 902~928
// TW TW 922~928
// CN CN 920~925
// CN2 CN2 840~845
// EU EU 865~868
// JP JP 916~921
// KR KR 917~921
// VN VN 918~923
typedef enum _RFID_REGULATION_ {
	REGULATION_UNKNOWN = 0,
	REGULATION_US = 1,  // US 902~928
	REGULATION_TW = 2,  // TW 922~928
	REGULATION_CN = 3,  // CN 920~925
	REGULATION_CN2 = 4, // CN2 840~845
	REGULATION_EU = 5,  // EU 865~868
	REGULATION_JP = 6,  // JP 916~921
	REGULATION_KR = 7,  // KR 917~921
	REGULATION_VN = 8,  // VN 918~923
} RFID_REGULATION,
	*PRFID_REGULATION;

typedef enum _RFID_MEMORY_BANK_ {
	RFID_MB_NONE = 0,
	RFID_MB_EPC = 1,  // EPC (Electronic Product Code)
	RFID_MB_TID = 2,  // TID (Tag Identifier)
	RFID_MB_USER = 3, // User Data
} RFID_MEMORY_BANK,
	*PRFID_MEMORY_BANK;

//#define POWER_VERSION
// Reader Version | Power(dbm)
// VC2	| - 2~18 dbm
// VD2  | - 2~25 dbm
// VD3  | 0~27 dbm
// VD4  | 2~29 dbm
// V6(TBD) |- 2~30 dbm

#pragma pack(1)
// SOFEWARE: VD407
// HARDWARE: CA
// READER ID : 000015E1
// RFID BAND : TW
// ModuleVersion : 5.5.20190704.1

typedef struct _RFID_READER_VERSION_ {
	TString strFirmware;          // Firmware Veasion
	TString strHardware;          // Hardware Version
	TString strReaderId;          // Reader ID
	TString strRfBandRegualation; // RF band regulation
	TString strMessage;           // Other Message

} RFID_READER_VERSION, *PRFID_READER_VERSION;

typedef struct _RFID_TAG_EPC_ {
	TString strRaw;                 // Raw Data
	unsigned int uiProtocolControl; // PC(Protocol Control )
	TString strEPC;                 // EPC
	unsigned int uiCRC16;           // CRC16
} RFID_TAG_EPC, *PRFID_TAG_EPC;

typedef struct _RFID_TAG_DATA_ {
	unsigned int uiAntenna; // Antenna
	TString strTime;        // yyyy/MM/dd HH:mm:ss:fff
	TString strData;        // [Option] Tag Bank Data
	TString strEPC;         // EPC
	TString strTID;         // TID
	TString strUser;        // User Data
	//--- Multi Tag ---
	TString strAccessPwd;   // Access Password
	TString strKillPwd;     // Kill Password
	TString strRSSI;        // RSSI
	TString strIP;          // Reader IP/COM
} RFID_TAG_DATA, *PRFID_TAG_DATA;

void print_epc(const RFID_TAG_EPC& tag);
void print_tag(const RFID_TAG_DATA& tag);




#if 0
typedef struct _RFID_TAG_EPC_
{
	int nAntenna;			// 天線
	TString strTime;		// yyyy/MM/dd HH:mm:ss:fff
	TString strData;		// Tag Bank資料
}RFID_TAG_EPC, *PRFID_TAG_EPC;
#endif

#if 0
typedef struct _RFID_TAG_
{
	unsigned int uiAntenna;			// Antenna
	TString strTime;		// 讀取時間 (yyyy/MM/dd HH:mm:ss:fff)
	TString strData;		// [Option] Tag Bank Data
	TString strEPC;			// EPC
	TString strTID;			// TID
	TString strUserData;	// @@@ User Memory @@@

	TString strAccessPwd;	// Access Password
	TString KillPwd;		// Kill Password
	TString strRSSI;		// RSSI
	TString strIP;			// Reader IP/COM
}RFID_TAG, *PRFID_TAG;
#endif

typedef struct _RFID_WRITE_RESULT_ {
	TString strStatus; // 0: success 1: failure
	TString strMessage;
} RFID_WRITE_RESULT, *PRFID_WRITE_RESULT;

typedef struct _RFID_LOCK_RESULT_ {
	TString strStatus; // 0: success 1: failure
	TString strMessage;
} RFID_LOCK_RESULT, *PRFID_LOCK_RESULT;

typedef struct _RFID_KILL_RESULT_ {
	TString strStatus; // 0: success 1: failure
	TString strMessage;
} RFID_KILL_RESULT, *PRFID_KILL_RESULT;

typedef struct _RFID_GPI_ {
	TString strPort;   // GPI Port
	TString strStatus; // 0/1: Low/High
	TString strIP;     // Reader IP/COM
} RFID_GPI, *PRFID_GPI;

#pragma pack()
