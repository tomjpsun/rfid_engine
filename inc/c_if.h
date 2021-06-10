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
#define HANDLE int

typedef enum _RFID_MEMORY_BANK_ {
	RFID_MB_NONE = 0,
	RFID_MB_EPC = 1,  // EPC (Electronic Product Code)
	RFID_MB_TID = 2,  // TID (Tag Identifier)
	RFID_MB_USER = 3, // User Data
} RFID_MEMORY_BANK,
	*PRFID_MEMORY_BANK;

extern "C"
{
	int RFModuleInit(char* config_path_name);
	HANDLE RFOpen(int index);
	//HANDLE RfidOpen(char* ip_addr, char ip_type, int port);
	int RFInventoryEPC(HANDLE h, int slot, bool loop, char **json_str, int* json_len);
	void RFSingleCommand(HANDLE h, char* userCmd, int userCmdLen, char **response_str, int* response_len);
	int RFReadMultiBank(HANDLE h, int slot, bool loop, int bankType,
			    int start, int wordLen, char **json_str, int* json_len);
	int RFSetSystemTime(HANDLE h);
	int RFReboot(HANDLE h);
	void RFClose(HANDLE h);
}
#endif //_C_IF_H_
