#ifndef _CPP_IF_HPP_
#define _CPP_IF_HPP_

#include <mutex>
#include <functional>
#include "packet_content.hpp"
#include "rfid_err.h"
#include "aixlog.hpp"
#include "rfid_config.hpp"
#include "parser.hpp"

using namespace std;

// AsyncCallbackFunc should alwayse return false,
// later the isEOP/isEOU capture will return true,
// which is the correct way to finish a command

using AsyncCallackFunc = function<bool(PacketContent pkt, void *user)>;
using HeartBeatCallackFunc = function<bool(std::string reader_id , void* user)>;
using FinishConditionType = function<bool(PacketContent pkt)>;
using HeartbeatCallbackType = function<void(std::string echo)>;

bool IsWatchDogEnabled();
void dumpDateTime(const struct tm& dTime);
int ReadBankHelper( HANDLE h, int slot, bool loop,
		    int bankType, int start, int wordLen,
		    vector<rfid::RfidParseUR>& results );
string RFReaderID(HANDLE h);
#endif // _CPP_IF_HPP_
