#ifndef _C_IF_H_
#define _C_IF_H_

#include "rfid_err.h"

#define IP_ADDR_LEN 16
#define IP_TYPE_IPV4 1
#define IP_TYPE_IPV6 2
#define MAX_PACKET_EXTRACT_COUNT 20
#define DEFAULT_SET_POWER_VALUE  10
#define RF_MODULE_RESET_TIME_MS  3000
// replied JSON was put on global buffer, with this limit
#define JSON_BUFFER_SIZE (1024*32)
#define MAX_HANDLE_SIZE (1000)
#define INVALID_HANDLE (-1)
#define EPC_LEN 64
#define TID_LEN 64
#define READER_ID_LEN 16
#define HANDLE int

typedef enum _RFID_MEMORY_BANK_ {
	RFID_MB_NONE = 0,
	RFID_MB_EPC = 1,  // EPC (Electronic Product Code)
	RFID_MB_TID = 2,  // TID (Tag Identifier)
	RFID_MB_USER = 3, // User Data
} RFID_MEMORY_BANK,
	*PRFID_MEMORY_BANK;

typedef enum _RFID_STATISTICS_RULE_ {
	RF_STATISTICS_RULE_BY_EPC = 0,
	RF_STATISTICS_RULE_BY_TID = 1,
	RF_STATISTICS_RULE_BY_EPC_OR_TID = 2,
} RF_STATISTICS_RULE;

extern "C"
{
	typedef struct _RFID_EPC_STATASTICS_ {
		char epc[EPC_LEN];
		char tid[TID_LEN];
		char readerID[READER_ID_LEN];
		int count;
		int antenna;
		int year;
		int month;
		int day;
		int hour;
		int min;
		int sec;
		int ms;
	} RFID_EPC_STATISTICS;

	int RFModuleInit();
	HANDLE RFOpen(int index);
	//HANDLE RfidOpen(char* ip_addr, char ip_type, int port);
	int RFInventoryEPC(HANDLE h, int slot, bool loop, char **json_str, int* json_len);
	void RFSingleCommand(HANDLE h, char* userCmd, int userCmdLen, char **response_str, int* response_len);
	int RFReadMultiBank(HANDLE h, int slot, bool loop, int bankType,
			    int start, int wordLen, char **json_str, int* json_len);
        int RFStatistics(HANDLE h, int slot, bool loop, int bankType, int start,
                         int wordLen, int reference_time,
                         RF_STATISTICS_RULE statistics_rule,
			 RFID_EPC_STATISTICS* stat_array, int* stat_count);

	int RFSetSystemTime(HANDLE h);
	int RFWriteEPC(HANDLE h, char* tid, int tid_len,
		       char* new_epc, int new_epc_len, bool double_check);
	int RFReboot(HANDLE h);
	int RFSetLoopTime(HANDLE h, unsigned int looptime_ms);
	int RFGetLoopTime(HANDLE h, unsigned int* looptime_ms);
	int RFSetLoopAntenna(HANDLE h, unsigned int antennas);
	int RFGetLoopAntenna(HANDLE h, unsigned int* antennas);
	int RFSetPower(HANDLE h, int nPower);
	int RFGetPower(HANDLE h, int* nPower);
	void RFClose(HANDLE h);
}
#endif //_C_IF_H_
