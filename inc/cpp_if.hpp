#ifndef _CPP_IF_HPP_
#define _CPP_IF_HPP_

#include <mutex>
#include <functional>
#include "packet_content.hpp"
#include "rfid_err.h"
#include "aixlog.hpp"
#include "rfid_config.hpp"

using namespace std;

using AsyncCallackFunc = function<bool(PacketContent pkt, void *user)>;
using HeartBeatCallackFunc = function<bool(std::string reader_id , void* user)>;
using FinishConditionType = function<bool(PacketContent pkt)>;
using HeartbeatCallbackType = function<void(std::string echo)>;

bool IsWatchDogEnabled();

#endif // _CPP_IF_HPP_
