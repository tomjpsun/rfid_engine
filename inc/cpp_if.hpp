#ifndef _CPP_IF_HPP_
#define _CPP_IF_HPP_

#include <mutex>
#include <functional>
#include "packet_content.hpp"
#include "rfid_err.h"
#include "aixlog.hpp"


using namespace std;

using AsyncCallackFunc = function<bool(PacketContent pkt, void *user)>;
using HeartBeatCallackFunc = function<bool(std::string reader_id , void* user)>;
using FinishConditionType = function<bool(PacketContent pkt)>;
using HANDLE=int;


extern "C"
{

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

        typedef struct _PQParams_ {
		char ip_addr[IP_ADDR_LEN];
		char ip_type; // IP_TYPE_IPV(4|6)
		int port; // default 1001
	}PQParams, *PPQParams;

	HANDLE RFOpen(PQParams* connection_settings);
	HANDLE RfidOpen(char* ip_addr, char ip_type, int port);
	int RFInventoryEPC(HANDLE h, int slot, bool loop, char **json_str, int* json_len);
	void RFSingleCommand(HANDLE h, char* userCmd, int userCmdLen, char **response_str, int* response_len);
	int RFReadMultiBank(HANDLE h, int slot, bool loop, int bankType,
			    int start, int wordLen, char **json_str, int* json_len);
	void RFClose(HANDLE h);
}

#endif // _CPP_IF_HPP_
