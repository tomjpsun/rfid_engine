#ifndef _RFID_IF_HPP_
#define _RFID_IF_HPP_

// rfid_if.hpp: AidiaLink RFID Interface Header for AL-510
//
//==============================================================================
// Header Name	: rfid_if.hpp
// Purpose      : RFID interface header for RFID reader
// Autor        : Richard Chuang
// Email        : richard_chung@davicom.com.tw
// Description  :
// Reference    :
// Copyright    : (c) Davicom Semiconductor Inc. All Rights Reserved.
// -----------------------------------------------------------------------------
// Version      : 1.0.0.0
// Update       : 2020-11-03
// Modified     : Richard Chung
// Description	:
// Version      : 1.1.0.0
// Update       : 2021-02-25
// Modified     : Tom Sun
// Description	: Porting from Windows to Linux
// -----------------------------------------------------------------------------
// How to use:
//
//-----------------------------------------------------------------------------
// Command overview
//-----------------------------------------------------------------------------
// 1. V: display reader firmware version
// 2. S : display reader ID
// 3. Q : display tag EPC ID
// 4. R : read tag memory data
// 5. W : write data to tag memory
// 6. K : kill tag
// 7. L : lock memory
// 8. P : set ACCESS password for ACCESS operation
// 9. U : Multi - TAG read EPC
// 10. G : logging command operation
// 11. T : Select matching tag
// 12. N0 / N1 : Read / Set RF power level
// 13. N4 / N5 : Read / Set RF Regulation
// 14. UR : Multi - TAG read EPC and read tag memory data
// 15. QR : Display tag EPC ID and read tag memory data
//
// Note:
// V S N command is for Reader information and parameter command.User can run in
// any time after reader power up. Q R W K L U T command is for TAG operation.In
// multi - TAG application, user can / must combination command for a TAG
// operation. In single TAG operation environment, user can do P Q R W K L
// command. In multi - TAG operation environment, only U command support multi -
// TAG Read EPC.Otherwise, user can or must combination T P U R W K L command
// for a single particular TAG operation.
//


#include <list>
#include <mutex>
#include <time.h>
#include <cstddef>
#include <functional>
#include "common.hpp"
#include "parse_ds.hpp"
#include "TStringTokenizer.h"
#include "conn_queue.hpp"
#include "packet_content.hpp"

using namespace std;

typedef int (*RFID_HOOK_PACKET_CALLBACK)(const void *lpPacket, int nSize,
                                         uint64_t usUserData);

typedef int (*RFID_ASYNC_PACKET_CALLBACK)(uint64_t uiID, unsigned int uiType,
                                          const void *lpPacket, int nSize,
                                          uint64_t usUserData);
typedef int (*RFID_HEARTBEAT_PACKET_CALLBACK)(uint64_t uiID,
                                              unsigned int uiType,
                                              const void *lpPacket, int nSize,
                                              uint64_t usUserData);

typedef struct _HOOK_PACKET_CALLBACK_ {
	RFID_HOOK_PACKET_CALLBACK fnCallback;
	uint64_t usUserData;
} HOOK_PACKET_CALLBACK, *PHOOK_PACKET_CALLBACK;

typedef struct _ASYNC_PACKET_CALLBACK_ {
	unsigned int uiType;
	RFID_ASYNC_PACKET_CALLBACK fnCallback;
	uint64_t usUserData;
} ASYNC_PACKET_CALLBACK, *PASYNC_PACKET_CALLBACK;

typedef struct _HEARTBEAT_PACKET_CALLBACK_ {
	unsigned int uiType;
	RFID_HEARTBEAT_PACKET_CALLBACK fnCallback;
	uint64_t usUserData;
} HEARTBEAT_PACKET_CALLBACK, *PHEARTBEAT_PACKET_CALLBACK;

typedef struct _RFID_READER_ANTENNA_ {
	uint64_t usAntenna;
} RFID_READER_ANTENNA, *PRFID_READER_ANTENNA;

typedef struct _RFID_RESPONSE_INFO_ {
	std::string strDate;
	uint64_t usAntenna;
} RFID_RESPONSE_INFO, *PRFID_RESPONSE_INFO;

inline bool operator==(const HOOK_PACKET_CALLBACK &lhs,
                       const HOOK_PACKET_CALLBACK &rhs) {
	return (lhs.fnCallback == rhs.fnCallback) &&
		(lhs.usUserData == rhs.usUserData);
};

inline bool operator==(const ASYNC_PACKET_CALLBACK &lhs,
                       const ASYNC_PACKET_CALLBACK &rhs) {
	return (lhs.fnCallback == rhs.fnCallback) &&
		(lhs.usUserData == rhs.usUserData);
};

inline bool operator==(const HEARTBEAT_PACKET_CALLBACK &lhs,
                       const HEARTBEAT_PACKET_CALLBACK &rhs) {
	return (lhs.fnCallback == rhs.fnCallback) &&
		(lhs.usUserData == rhs.usUserData);
};

inline bool operator==(const RFID_READER_ANTENNA &lhs,
                       const RFID_READER_ANTENNA &rhs) {
	return (lhs.usAntenna == rhs.usAntenna);
}

inline bool operator==(const RFID_RESPONSE_INFO &lhs,
                       const RFID_RESPONSE_INFO &rhs) {
	return (lhs.strDate == rhs.strDate) && (lhs.usAntenna == rhs.usAntenna);
}

inline bool operator==(const struct tm &lhs,
		       const struct tm &rhs) {
	int* p = (int *)&lhs;
	int* q = (int *)&rhs;
	bool result  = true;
	for (int i = 0;
	     i < (int)(offsetof(struct tm, tm_wday)/sizeof(int));
	     i++, p++, q++) {
		result = result && (*p == *q);
	}
	return result;
}

inline void print_tm(string label, const struct tm& time)
{
	// The Reader Device only reply partial fields of struct tm
	LOG(SEVERITY::DEBUG)
		<< label << ": "
		<< ", sec: "  << time.tm_sec
		<< ", min: "  << time.tm_min
		<< ", hour: " << time.tm_hour
		<< ", month day: "  << time.tm_mday
		<< ", month: "<< time.tm_mon + 1
		<< ", year: " << time.tm_year + 1900
		//<< ", week day: " << time.tm_wday
		//<< ", year day: " << time.tm_yday
		//<< ", dst: " << time.tm_isdst
		<< endl;
}

typedef enum _RFID_SESSION_ {
	RFID_SESSION_NONE = -1,
	RFID_SESSION_0 = 0,
	RFID_SESSION_1 = 1,
	RFID_SESSION_2 = 2,
	RFID_SESSION_3 = 3,
} RFID_SESSION,
	*PRFID_SESSION;

typedef enum _RFID_TARGET_ {
	RFID_TARGET_A = 0,
	RFID_TARGET_B = 1
} RFID_TARGET,
	*PRFID_TARGET;


class RfidInterface {
public:
	RfidInterface();
	~RfidInterface();

public:
	//------------------------------------------------------------------------------
	// APIs
	//------------------------------------------------------------------------------
	virtual bool GetVersion(RFID_READER_VERSION &stVersion);
	virtual bool GetModuleVersion(TString &strVersion);
	virtual bool GetReaderID(TString &strID);
	virtual bool SetRegulation(RFID_REGULATION emRegulation);
	virtual bool GetRegulation(RFID_REGULATION &emRegulation);
	//------------------------------------------------------------------------------
	//
	//------------------------------------------------------------------------------
	virtual bool SetPower(int nPower, int *pnResult); // Power(dbm)
	virtual bool GetPower(int &nPower);               // Power(dbm)
	virtual bool SetSingleAntenna(unsigned int uiAntenna, bool fHub,
				      unsigned int *pnResult);
	virtual bool GetSingleAntenna(unsigned int &uiAntenna, bool &fHub);
	virtual bool SetLoopAntenna(unsigned int uiAntennas);
	virtual bool GetLoopAntenna(unsigned int &uiAntennas);
	virtual bool SetLoopTime(unsigned int uiMilliseconds);
	virtual bool GetLoopTime(unsigned int &uiMilliseconds);
	virtual bool SetSystemTime();           // SetSysNowTime
	virtual bool SetTime(struct tm stTime); // yyMMddHHmmss
	virtual bool GetTime(struct tm &stTime);
	//------------------------------------------------------------------------------
	// Single Antenna APIs
	//------------------------------------------------------------------------------
	// bool ReadEPC(RFID_TAG_DATA &stTagData);
	virtual bool ReadBank(RFID_BANK_TYPE emType, int nStart, int nLength);
	virtual bool ReadBank(RFID_MEMORY_BANK emBank, unsigned int uiStartAddress,
			      unsigned int uiWordLength, RFID_TAG_DATA &stTagData,
			      unsigned int *puiErrorCode = NULL);
	virtual bool ReadUserData(unsigned int uiStartAddress,
				  unsigned int uiWordLength, RFID_TAG_DATA &stTagData,
				  unsigned int *puiErrorCode = NULL);
	// bool ReadTag(RFID_MEMORY_BANK emBank, unsigned int uiStartAddress, unsigned
	// int uiWordLength, RFID_TAG_DATA &stTagData, unsigned int * puiErrorCode =
	// NULL);
	virtual bool ReadEPC(unsigned int uiStartAddress, unsigned int uiWordLength,
			     RFID_TAG_DATA &stTagData,
			     unsigned int *puiErrorCode = NULL);
	virtual bool ReadTID(unsigned int uiStartAddress, unsigned int uiWordLength,
			     RFID_TAG_DATA &stTagData,
			     unsigned int *puiErrorCode = NULL);
	virtual bool ReadEPCandTID(unsigned int uiStartAddress,
				   unsigned int uiWordLength,
				   RFID_TAG_DATA &stTagData,
				   RFID_TAG_EPC *pstTagEPC = NULL,
				   unsigned int *puiErrorCode = NULL);
	virtual bool ReadEPCandUserData(unsigned int uiStartAddress,
					unsigned int uiWordLength,
					RFID_TAG_DATA &stTagData,
					RFID_TAG_EPC *pstTagEPC = NULL,
					unsigned int *puiErrorCode = NULL);

	virtual bool ReadSingleTagEPC(bool fLoop = false);
	virtual bool ReadMultiTagEPC(int nSlot,
				     bool fLoop = false); // Multi-TAG read EPC
	virtual bool ReadMultiBank(RFID_BANK_TYPE emType, int nStart, int nLength,
				   bool fLoop = false); // Multi-TAG read EPC and data

	//------------------------------------------------------------------------------
	// Inventory APIs
	//------------------------------------------------------------------------------
	virtual bool SelectMatchingTag(RFID_TAG_DATA &stTagData);
	// virtual bool Inventory(RFID_TAG_DATA &stTagData);

	virtual bool SetSession(RFID_SESSION emSession, RFID_TARGET emTarget);
	// virtual bool GetSession(unsigned int &uiSession, RFID_TARGET &emTarget);

	virtual bool
	InventoryEPC(int nSlotQ, bool fLoop, int repeat = 1);
	// bool Inventory(int nSlotQ, SessionType session, TargetType target);
	virtual bool
	InventoryTID(int nSlotQ,
		     RFID_TAG_DATA &stTagData); // Inventory Tag EPC and TID
	// bool InventoryTID(int nSlotQ, RFID_TAG &stTag, SessionType session,
	// TargetType target);
	virtual bool InventoryUser(int nSlotQ,
				   int nLength); // Inventory Tag EPC and User Memory
	// bool InventoryUser(int nSlotQ, int nLength, SessionType session, TargetType
	// target);
	//------------------------------------------------------------------------------
	//
	//------------------------------------------------------------------------------
	virtual bool
	WriteEPC(); // string pwd, string data, BankType targetBank, string mask
	virtual bool WriteBank(); // BankType type, int start, string pwd, string
	// data, BankType targetBank, string mask
	virtual bool LockTag(); // BankType bank, LockType locktype, string pwd,
	// BankType targetbank, string mask
	virtual bool KillTag(); // string killPwd, BankType targetBank, string mask
	//------------------------------------------------------------------------------
	// GPIO
	//------------------------------------------------------------------------------
	virtual bool SetGPO(int nPort, bool fIsOn);
	virtual bool GetGPO(); // bool[] GetGPO()
	virtual bool GetGPI(); // bool[] GetGPI()
	virtual bool GPI(RFID_GPI &stGPI);
	//------------------------------------------------------------------------------
	//
	//------------------------------------------------------------------------------
	virtual bool SetUSBHID();
	virtual bool SetUSBKeyboard();
	//------------------------------------------------------------------------------
	// WifiAP
	//------------------------------------------------------------------------------
	virtual bool SetWifiAP();
	virtual bool GetWifiAP();
	virtual bool SetWifiStaticIP();
	virtual bool GetWifiStaticIP();
	virtual bool SetDHCPEnable();
	virtual bool SetDHCPDisable();
	virtual bool GetDHCPStatus();
	virtual bool GetWifiIP();
	virtual bool GetWifiAPInfo();
	//------------------------------------------------------------------------------
	//
	//------------------------------------------------------------------------------
	virtual bool OpenHeartbeat(unsigned int uiMilliseconds);
	virtual bool CloseHeartbeat();
	//------------------------------------------------------------------------------
	//
	//------------------------------------------------------------------------------
	virtual bool FlashEMTag();
	virtual bool GetEMTemprature();
	virtual bool GetEMBatteryVoltage();

	//------------------------------------------------------------------------------
	// Hook Packet Callback
	//------------------------------------------------------------------------------
	bool RegisterHookCallback(RFID_HOOK_PACKET_CALLBACK fnHookPacketCallback,
				  uint64_t usUserData);
	void UnRegisterHookCallback(RFID_HOOK_PACKET_CALLBACK fnHookPacketCallback,
				    uint64_t usUserData);
	bool OnHookCallback(const void *lpPacket, int nSize);
	//------------------------------------------------------------------------------
	// Async Packet Callback
	//------------------------------------------------------------------------------
	bool RegisterAsyncCallback(RFID_ASYNC_PACKET_CALLBACK fnAsyncPacketCallback,
				   uint64_t usUserData, unsigned int uiType = 0);
	void UnRegisterAsyncCallback(RFID_ASYNC_PACKET_CALLBACK fnAsyncPacketCallback,
				     uint64_t usUserData);
	bool OnAsyncCallback(uint64_t uiID, unsigned int uiType, const void *lpPacket,
			     int nSize);
	//------------------------------------------------------------------------------
	// Heartbeat Packet Callback
	//------------------------------------------------------------------------------
	bool RegisterHeartbeatCallback(
		RFID_HEARTBEAT_PACKET_CALLBACK fnHeartbeatPacketCallback,
		uint64_t usUserData);
	void UnRegisterHeartbeatCallback(
		RFID_HEARTBEAT_PACKET_CALLBACK fnHeartbeatPacketCallback,
		uint64_t usUserData);
	bool OnHeartbeatCallback(uint64_t uiID, unsigned int uiType,
				 const void *lpPacket, int nSize);

protected:
	//==============================================================================
	// Parse
	//==============================================================================
	bool GetContent(const void *lpBuffer, int nBufferLength, TString &strContent,
			bool fRemoveLeadingWord = true);
	//------------------------------------------------------------------------------
	//
	//------------------------------------------------------------------------------
	bool ParseVersion(const void *lpBuffer, int nBufferLength,
			  RFID_READER_VERSION &stVersion);
	bool ParseModuleVersion(const void *lpBuffer, int nBufferLength,
				TString &strVersion);
	bool ParseReaderID(const void *lpBuffer, int nBufferLength, TString &strID);
	// bool ParseRegulation(const void* lpBuffer, int nBufferLength,
	// RFID_REGULATION &emRegulation);
	bool ParseSetRegulation(const void *lpBuffer, int nBufferLength,
				int *pnResult);
	bool ParseGetRegulation(const void *lpBuffer, int nBufferLength,
				RFID_REGULATION &emRegulation);
	bool ParseGetPower(const void *lpBuffer, int nBufferLength, int &nValue);
	bool ParseSetPower(const void *lpBuffer, int nBufferLength, int *pnResult);

	bool ParseSetSingleAntenna(const void *lpBuffer, int nBufferLength,
				   unsigned int *puiAntenna, bool *pfHub);
	bool ParseGetSingleAntenna(const void *lpBuffer, int nBufferLength,
				   unsigned int &uiAntenna, bool &fHub);
	bool ParseSetLoopAntenna(const void *lpBuffer, int nBufferLength,
				 unsigned int *pnResult);
	bool ParseGetLoopAntenna(const void *lpBuffer, int nBufferLength,
				 unsigned int &uiAntennas);
	bool ParseSetLoopTime(const void *lpBuffer, int nBufferLength,
			      unsigned int *pnResult);
	bool ParseGetLoopTime(const void *lpBuffer, int nBufferLength,
			      unsigned int &uiMilliseconds);

	bool ParseSetTime(const void *lpBuffer, int nBufferLength,
			  struct tm *pstTime); // Note: tm_yday: Millisecond after
	// second  (0 - 999).
	bool ParseGetTime(
		const void *lpBuffer, int nBufferLength,
		struct tm &stTime); // Note: tm_yday: Millisecond after second  (0 - 999).

	bool ParseReadBank(const void *lpBuffer, int nBufferLength,
			   RFID_TAG_DATA &stTagData,
			   unsigned int *puiErrorCode = NULL);
	// bool ParseReadTag(const void* lpBuffer, int nBufferLength, RFID_TAG_DATA
	// &stTagData, unsigned int * puiErrorCode = NULL);

	bool ParseReadEPC(const void *lpBuffer, int nBufferLength,
			  RFID_TAG_DATA &stTagData,
			  unsigned int *puiErrorCode = NULL);
	bool ParseReadTID(const void *lpBuffer, int nBufferLength,
			  RFID_TAG_DATA &stTagData,
			  unsigned int *puiErrorCode = NULL);
	bool ParseReadUserData(const void *lpBuffer, int nBufferLength,
			       RFID_TAG_DATA &stTagData,
			       unsigned int *puiErrorCode = NULL);
	bool ParseReadEPCandTID(const void *lpBuffer, int nBufferLength,
				RFID_TAG_DATA &stTagData,
				RFID_TAG_EPC *pstTagEPC = NULL,
				unsigned int *puiErrorCode = NULL);
	bool ParseReadEPCandUserData(const void *lpBuffer, int nBufferLength,
				     RFID_TAG_DATA &stTagData,
				     RFID_TAG_EPC *pstTagEPC = NULL,
				     unsigned int *puiErrorCode = NULL);
	//------------------------------------------------------------------------------
	// Inventory: Select matching tag
	//------------------------------------------------------------------------------
	bool ParseReadSingleEPC(const void *lpBuffer, int nBufferLength,
				RFID_TAG_DATA &stTagData,
				RFID_TAG_EPC *pstTagEPC = NULL,
				unsigned int *puiErrorCode = NULL);

	bool ParseInventory(const void *lpBuffer, int nBufferLength,
			    RFID_TAG_DATA &stTagData, RFID_TAG_EPC *pstTagEPC = NULL,
			    unsigned int *puiErrorCode = NULL);
	bool ParseReadMultiEPC(const void *lpBuffer, int nBufferLength,
			       RFID_TAG_DATA &stTagData,
			       RFID_TAG_EPC *pstTagEPC = NULL,
			       unsigned int *puiErrorCode = NULL);

	bool ParseSelectMatching(const void *lpBuffer, int nBufferLength,
				 RFID_TAG_DATA &stTagData);
	bool ParseSession(const void *lpBuffer, int nBufferLength,
			  RFID_TAG_DATA &stTagData);

protected:
	//------------------------------------------------------------------------------
	// Others
	//------------------------------------------------------------------------------
	bool ParseAntenna(const void *lpszAntenna, unsigned int &uiAntenna);
	bool ParseAntenna(const void *lpszAntenna, unsigned int &uiAntenna,
			  bool &fHub);
	bool ParseDateTime(
		string strDataTime,
		struct tm &stTime); // Note: tm_yday: Millisecond after second  (0 - 999).

	unsigned int ParseErrorCode(TCHAR szResponseCode);
	int RegulatePower(int nPower);
        //------------------------------------------------------------------------------
	//==============================================================================
	// Socket
	//==============================================================================
	virtual int Receive(unsigned int &uiPacketType, void *lpBuf, int nBufLen,
			    int nFlags = 0);
	virtual int Send(unsigned int uiPacketType, const void *lpBuf, int nBufLen,
			 int nFlags = 0);
	int AsyncSend(unsigned int uiPacketType, void *lpBuf,
		      int nBufLen, AsyncCallackFunc callback, void* user, int nFlags);

	vector<FinishConditionType> CompileFinishConditions(unsigned int uiPacketType);

        // Generate custom packets
	int GeneratePacket(unsigned char *lpbyBuffer, uint64_t uiBufferSize,
			   unsigned int uiPacketType, unsigned char *lpbyOriginal,
			   uint64_t uiOriginalSize, bool fSend);

protected:
	//------------------------------------------------------------------------------
	// Packet Hook Callback
	//------------------------------------------------------------------------------
	std::mutex m_muxHookLock;      // critical section
	std::mutex m_muxAsyncLock;     // critical section
	std::mutex m_muxHeartbeatLock; // critical section

	list<HOOK_PACKET_CALLBACK> m_lstHookCallback;
	list<ASYNC_PACKET_CALLBACK> m_lstAsyncCallback;
	list<HEARTBEAT_PACKET_CALLBACK> m_lstHeartbeatCallback;
	//------------------------------------------------------------------------------
	// Send / Recv Command
	//------------------------------------------------------------------------------
	unsigned int m_uiSendCommand; // The last send command
	unsigned int m_uiRecvCommand; // The last received command

	//------------------------------------------------------------------------------
	// Reader Unique Identification ID
	//------------------------------------------------------------------------------
	uint64_t m_uiDeviceID; // Unique Identification ID / Unique Reader ID
	RFID_READER_VERSION version_info;
	TStringTokenizer m_objTokenizer;
	ConnQueue<PacketContent> conn_queue;
};
#endif // _RFID_IF_HPP_
