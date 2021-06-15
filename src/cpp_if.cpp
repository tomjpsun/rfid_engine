#include "c_if.h"
#include "cpp_if.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <chrono>
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
using namespace std::chrono;

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


int RFModuleInit(char* config_path_name)
{
	int result = RFID_OK;
	std::ifstream i(config_path_name);
	if ( !i.good() ) {
		cout << config_path_name << " file not found" << endl;
		result = RFID_ERR_FILE_NOT_EXIST;
	} else {
		json j;
		i >> j;
		g_cfg = j;
		if ( ! is_log_init_ed ) {
			auto sink_cout = make_shared<AixLog::SinkCout>( LogLevelMap[g_cfg.log_level] );
			auto sink_file = make_shared<AixLog::SinkFile>( LogLevelMap[g_cfg.log_level], g_cfg.log_file);
			AixLog::Log::init({sink_cout, sink_file});
			is_log_init_ed = true;
		}
	}
	LOG(SEVERITY::NOTICE) << "version :"
			      << MAJOR << "."
			      << MINOR << "."
			      << SUB_MINOR << endl;
	return result;
}



HANDLE RFOpen(int index)
{
	LOG(SEVERITY::DEBUG) << "index = " << index << endl;
	ReaderInfo info = g_cfg.reader_info_list[index];

	shared_ptr<RfidInterface> prf =
		shared_ptr<RfidInterface>(new RfidInterface(info));

	int r = hm.add_handle_unit(prf);
	LOG(SEVERITY::DEBUG) << "return handle = " << r << endl;
	return r;
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

int
ReadBankHelper( HANDLE h, int slot, bool loop,
		int bankType, int start, int wordLen,
		vector<RfidParseUR>& results ) {

	vector<string> read_mb;
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
				results.push_back(parseUR);
		}
		ret = RFID_OK;
	}
	return ret;
}

int RFReadMultiBank(HANDLE h, int slot, bool loop, int bankType,
		    int start, int wordLen, char **json_str, int* json_len) {
	vector<string> read_mb;
	vector<RfidParseUR> convert;
	int ret;

        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		ret = ReadBankHelper( h, slot, loop, bankType,
				      start, wordLen, convert);
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

int RFReboot(HANDLE h)
{
	int ret;
        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		ret = hm.get_rfid_ptr(h)->Reboot();
        }
        return ret;
}


void DoStatisticHelper( vector<RfidParseUR>& reader_result,
		   std::map<string, int>& stat_result ) {

	for (RfidParseUR& parse : reader_result) {
		if (parse.has_data && parse.epc.is_match) {
			string epc = parse.epc.epc;
			if ( stat_result.count(epc) )
				stat_result[epc] = stat_result[epc] + 1;
			else
				stat_result[epc] = 1;
				//LOG(SEVERITY::TRACE) << "current epc = " << epc << ", count = " << stat_result[epc] << endl;
		}
	}
}


/* RFStatistics()
     Input:
       h:        handle
       slot:     batch size for each read (2^slot)
       loop:     if 'R' command should bind with 'U'
       bankType: RFID_MB_EPC or RFID_MB_TID or RFID_MB_USER
       start:    start address in tag on reading
       wordLen:  word length of reading
       reference_time: repeat read duration in millisec
       buffer:   buffer to loads the statistics results
       stat_count: max units of buffer
     Output:
       buffer:   buffer with the statistics results
       stat_count: effect number of units in buffer
 */
int RFStatistics(HANDLE h, int slot, bool loop, int bankType,
		 int start, int wordLen, int reference_time,
		 RFID_EPC_STATISTICS* buffer, int* stat_count)
{
	vector<string> read_mb;
	vector<RfidParseUR> reader_result;
	int ret = RFID_OK;
	std::map<string, int> stat_result;

	// check input buffer/size
	if (buffer == nullptr || stat_count == 0)
		return RFID_ERR_INVALID_BUFFER;

	// clear input buffer
	std::memset( buffer, 0, (*stat_count) * sizeof(RFID_EPC_STATISTICS) );
        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		auto time_start = steady_clock::now();

		while (true) {
			ret = ReadBankHelper( h, slot, loop, bankType,
					      start, wordLen, reader_result);
			auto time_stop = steady_clock::now();
			std::chrono::duration<double, std::milli> du = time_stop - time_start;
			LOG(SEVERITY::DEBUG) << "elapsed time: " << du.count() << " ms" << endl;
			if (du.count() > reference_time)
				break;
		};

		DoStatisticHelper(reader_result, stat_result);

                // fill output buffer
		RFID_EPC_STATISTICS* p_stat = buffer;
		int max_buffer_unit = *stat_count;
		int n_items = 0;
		for ( std::tuple<string, int> tup : stat_result ) {
			std::memcpy(p_stat->epc, std::get<0>(tup).c_str(), EPC_LEN);
			p_stat->count = std::get<1>(tup);
			n_items++;
			p_stat++;
			if (n_items >= max_buffer_unit) {
				ret = RFID_ERR_BUFFER_OVERFLOW;
				break;
			}
		}
		*stat_count = n_items;
	}
	return ret;
}


void RFClose(HANDLE h)
{
	hm.remove_handle_unit(h);
}
