#ifndef _C_IF_H_
#define _C_IF_H_

#include "rfid_err.h"
#include "time.h"

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
#define DEVICE_NAME_LEN 20
#define HANDLE int



#define RF_HUB_1_ANTENNA_1								0x00000001	// Antenna1-1
#define RF_HUB_2_ANTENNA_1								0x00000002	// Antenna1-2
#define RF_HUB_3_ANTENNA_1								0x00000004	// Antenna1-3
#define RF_HUB_4_ANTENNA_1								0x00000008	// Antenna1-4
#define RF_HUB_5_ANTENNA_1								0x00000010	// Antenna1-5
#define RF_HUB_6_ANTENNA_1								0x00000020	// Antenna1-6
#define RF_HUB_7_ANTENNA_1								0x00000040	// Antenna1-7
#define RF_HUB_8_ANTENNA_1								0x00000080	// Antenna1-8
#define RF_HUB_1_ANTENNA_2								0x00000100	// Antenna2-1
#define RF_HUB_2_ANTENNA_2								0x00000200	// Antenna2-2
#define RF_HUB_3_ANTENNA_2								0x00000400	// Antenna2-3
#define RF_HUB_4_ANTENNA_2								0x00000800	// Antenna2-4
#define RF_HUB_5_ANTENNA_2								0x00001000	// Antenna2-5
#define RF_HUB_6_ANTENNA_2								0x00002000	// Antenna2-6
#define RF_HUB_7_ANTENNA_2								0x00004000	// Antenna2-7
#define RF_HUB_8_ANTENNA_2								0x00008000	// Antenna2-8
#define RF_HUB_1_ANTENNA_3								0x00010000	// Antenna3-1
#define RF_HUB_2_ANTENNA_3   							        0x00020000	// Antenna3-2
#define RF_HUB_3_ANTENNA_3								0x00040000	// Antenna3-3
#define RF_HUB_4_ANTENNA_3								0x00080000	// Antenna3-4
#define RF_HUB_5_ANTENNA_3								0x00100000	// Antenna3-5
#define RF_HUB_6_ANTENNA_3								0x00200000	// Antenna3-6
#define RF_HUB_7_ANTENNA_3								0x00400000	// Antenna3-7
#define RF_HUB_8_ANTENNA_3								0x00800000	// Antenna3-8
#define RF_HUB_1_ANTENNA_4								0x01000000	// Antenna4-1
#define RF_HUB_2_ANTENNA_4								0x02000000	// Antenna4-2
#define RF_HUB_3_ANTENNA_4								0x04000000	// Antenna4-3
#define RF_HUB_4_ANTENNA_4   							        0x08000000	// Antenna4-4
#define RF_HUB_5_ANTENNA_4								0x10000000	// Antenna4-5
#define RF_HUB_6_ANTENNA_4								0x20000000	// Antenna4-6
#define RF_HUB_7_ANTENNA_4								0x40000000	// Antenna4-7
#define RF_HUB_8_ANTENNA_4								0x80000000	// Antenna4-8

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

	enum ReaderSettingsConnectionType { SOCKET=0, SERIAL=1 };
	struct ReaderSettings
	{
		ReaderSettingsConnectionType type;
		int ipv4[4];
		int ipv6[6];
		char dev_name[DEVICE_NAME_LEN];
		int port;
		int power;
		int loop_time;
		int antennas[4];
		int hub_number;
		char reader_id[READER_ID_LEN];
	};

	int RFModuleInit();
	HANDLE RFOpen(ReaderSettings rs);
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
	int RFGetTime(HANDLE h, struct tm& stTime);
	int RFSetTime(HANDLE h, struct tm stTime);

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

	inline int RFAntennaArrayToValue(int antenna_ary[], int count)
	{
		const int MapCount = 4;
		int map[MapCount] = {
			RF_HUB_1_ANTENNA_1,
			RF_HUB_1_ANTENNA_2,
			RF_HUB_1_ANTENNA_3,
			RF_HUB_1_ANTENNA_4
		};
		int result = 0;
		for (int i=0; i<count; i++) {
			int antenna_index = antenna_ary[i];
			if (antenna_index <= MapCount && antenna_index >=0)
				result |= map[antenna_ary[i] - 1];
		}
		return result;
	}

}


#endif //_C_IF_H_
