#include "cpp_if.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <climits>
#include <condition_variable>
#include <vector>
#include <memory>
#include <map>
#include "common.hpp"
#include "cmd_handler.hpp"
#include "packet_content.hpp"
#include "observer.hpp"
#include "aixlog.hpp"
#include "send_sync_observer.hpp"
#include "rfid_if.hpp"
#include "rfid_err.h"
#include "parser.hpp"
#include "nlohmann/json.hpp"
#include "handle_manager.hpp"
#include "rfid_config.hpp"


using namespace std;
using namespace rfid;
using json = nlohmann::json;


// holds obj which user had opened, erase on user close it
static HandleManager hm{};

static map<int, AixLog::Severity> LogLevelMap = {
    { 0, AixLog::Severity::trace   },
    { 1, AixLog::Severity::debug   },
    { 2, AixLog::Severity::info    },
    { 3, AixLog::Severity::notice  },
    { 4, AixLog::Severity::warning },
    { 5, AixLog::Severity::error   },
    { 6, AixLog::Severity::fatal   }
};

// init all to 0
RfidConfig cfg{};

HANDLE RFOpen(int index)
{
        static bool is_log_init_ed = false;

	std::ifstream i("rfid_config.json");
	json j;
	i >> j;
	RfidConfig cfg = j;

        // work-around, nlohmann cannot convert
	// member of vector type directly
        json j2 = j["reader_info_list"];
	vector<ReaderInfo> info_list = j2;

	if ( ! is_log_init_ed ) {
		auto sink_cout = make_shared<AixLog::SinkCout>( LogLevelMap[cfg.log_level] );
		auto sink_file = make_shared<AixLog::SinkFile>( LogLevelMap[cfg.log_level], cfg.log_file);
		AixLog::Log::init({sink_cout, sink_file});
		is_log_init_ed = true;
	}

	ReaderInfo info = info_list[index];
        PQParams params;
	params.ip_type = IP_TYPE_IPV4;
	params.port = info.reader_port;
        snprintf(params.ip_addr, IP_ADDR_LEN, "%s", info.reader_ip.c_str());

	shared_ptr<RfidInterface> prf =
		shared_ptr<RfidInterface>(new RfidInterface(params));

	return hm.add_handle_unit(prf);
}

#if 0
HANDLE RfidOpen(char *ip_addr, char ip_type, int port) {
	PQParams params;
	params.ip_type = ip_type;
	params.port = port;
	snprintf(params.ip_addr, IP_ADDR_LEN, "%s", ip_addr);
	return RFOpen(&params);
}
#endif

int RFInventoryEPC(HANDLE h, int slot, bool loop, char **json_str, int* json_len)
{
	int ret;
	vector<string> responses;
	vector<RfidParseU> convert;

        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		ret = hm.get_rfid_ptr(h)->InventoryEPC(slot, loop, responses);
		for (auto& response : responses) {
			convert.push_back( RfidParseU {response} );
		}
		nlohmann::json j = convert;
		string s = j.dump();

		hm.clear_buffer(h);
		hm.append_data(h, s);
		*json_str = hm.get_data(h, json_len);
		LOG(SEVERITY::TRACE) << "json str = " << *json_str << endl;
	}
        return ret;
}


void RFSingleCommand(HANDLE h, char* userCmd, int userCmdLen, char **response_str, int* response_len)
{

	string response;

        if ( !hm.is_valid_handle(h) ) {
		LOG(SEVERITY::ERROR) << " RFID_ERR_INVALID_HANDLE " << endl;
	} else {
		string user(userCmd, userCmdLen);
		response = hm.get_rfid_ptr(h)->SingleCommand(user);
		hm.clear_buffer(h);
		hm.append_data(h, response);
		*response_str = hm.get_data(h, response_len);
		LOG(SEVERITY::TRACE) << "response = " << *response_str << endl;
	}
}


int RFReadMultiBank(HANDLE h, int slot, bool loop, int bankType,
		    int start, int wordLen, char **json_str, int* json_len) {
	vector<string> read_mb;
	vector<RfidParseUR> convert;
	int err = 0;
	int ret;

        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		ret = hm.get_rfid_ptr(h)->ReadMultiBank(slot, loop, (RFID_MEMORY_BANK)bankType,
							start, wordLen, read_mb, err);
		for (auto response : read_mb) {
			RfidParseUR parseUR(response, bankType);
			convert.push_back(parseUR);
		}
		nlohmann::json j = convert;
		string s = j.dump();

		hm.clear_buffer(h);
		hm.append_data(h, s);
		*json_str = hm.get_data(h, json_len);
		LOG(SEVERITY::TRACE) << "json str = " << *json_str << endl;
	}
	return ret;
}



void RFClose(HANDLE h)
{
	hm.remove_handle_unit(h);
}
