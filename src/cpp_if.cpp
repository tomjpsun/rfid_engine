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

static bool is_log_init_ed = false;
// init all to 0
RfidConfig g_cfg{};


bool RFGetConfig(RfidConfig& cfg)
{
	if (is_log_init_ed)
		cfg = g_cfg;
	else
		cout << "not module init yet" << endl;
	return is_log_init_ed;
}


int RFModuleInit(char* config_path_name)
{
	int result = RFID_OK;
	std::ifstream i(config_path_name);
	if ( !i.good() ) {
		cout << config_path_name << " file not found" << endl;
		result = RFID_ERR_FILE_NOT_EXIST;
	} else {
		if ( ! is_log_init_ed ) {
			auto sink_cout = make_shared<AixLog::SinkCout>( LogLevelMap[g_cfg.log_level] );
			auto sink_file = make_shared<AixLog::SinkFile>( LogLevelMap[g_cfg.log_level], g_cfg.log_file);
			AixLog::Log::init({sink_cout, sink_file});
			is_log_init_ed = true;
		}
		json j;
		i >> j;
		g_cfg = j;
	}

	return result;
}



HANDLE RFOpen(int index)
{
	ReaderInfo info = g_cfg.reader_info_list[index];
        PQParams params;
	params.ip_type = IP_TYPE_IPV4;
	params.port = info.reader_port;
        snprintf(params.ip_addr, IP_ADDR_LEN, "%s", info.reader_ip.c_str());

	shared_ptr<RfidInterface> prf =
		shared_ptr<RfidInterface>(new RfidInterface(params));

	return hm.add_handle_unit(prf);
}


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
			RfidParseU parseU(response);
			if (parseU.is_match && parseU.has_data)
				convert.push_back( parseU );
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
			if (parseUR.is_match && parseUR.has_data)
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


int RFSetSystemTime(HANDLE h)
{
	int ret;
        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		ret = hm.get_rfid_ptr(h)->SetSystemTime();
	}
	return ret;
}


void RFClose(HANDLE h)
{
	hm.remove_handle_unit(h);
}
