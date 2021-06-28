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
#include <cstring>
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


// ========== Helper functions ==========

bool IsWatchDogEnabled()
{
	return g_cfg.enable_watch_dog;
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


void DoStatisticHelper(vector<RfidParseUR> &reader_result,
		       vector<RFID_EPC_STATISTICS>& stat_result)
{
	// from reader_result, collect count on each EPC,
	// and write to stat_result

	for (RfidParseUR& parse : reader_result) {
		if (parse.has_data && parse.epc.is_match) {
			string epc = parse.epc.epc;
			int antenna = std::stoi( parse.antenna );
			vector<RFID_EPC_STATISTICS>::iterator p =
				std::find_if(stat_result.begin(),
					     stat_result.end(),
					     [&epc, &antenna](vector<RFID_EPC_STATISTICS>::value_type& item) {
						     return (item.epc == epc) &&
							     (item.antenna == antenna);
					     });
			if ( p == stat_result.end() ) {
				RFID_EPC_STATISTICS new_stat;
				std::memcpy(new_stat.epc, epc.c_str(), EPC_LEN);
				new_stat.antenna = antenna;
				new_stat.count = 1;
				stat_result.push_back(new_stat);
			}
			else {
				p->count += 1;
			}
			//LOG(SEVERITY::TRACE) << "current epc = " << epc << ", count = " << stat_result[epc] << endl;
		}
	}
}


// ========== API functions ==========

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
	ReaderInfo info = g_cfg.reader_info_list[index];
	LOG(SEVERITY::DEBUG) << "index = " << index << ", ip = " << info.settings[0] << endl;

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


int RFWriteEPC(HANDLE h, char* tid, int tid_len,
	       char* new_epc, int new_epc_len, bool double_check)
{
	string epc(new_epc, new_epc_len);
	string ref_tid(tid, tid_len);
	int ret = RFID_ERR_OTHER;

        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		int start = 0;
		int n_word = 6;

		// SelectTag using bits as length
		hm.get_rfid_ptr(h)->SelectTag( RFID_MB_TID, start, n_word << 4 , ref_tid );

		// use empty here, password version not yet implemented
		std::string pass;
		hm.get_rfid_ptr(h)->Password(pass);

		ret = hm.get_rfid_ptr(h)->WriteBank( RFID_MB_EPC, 2, n_word, epc );
		if ( double_check ) {
			vector<string> read_mb;
			int err = 0;
			hm.get_rfid_ptr(h)->ReadMultiBank(3, true, RFID_MB_TID, start, n_word, read_mb, err);
			for (auto reply_str : read_mb) {
				RfidParseUR parseUR(reply_str, RFID_MB_TID);
				if (parseUR.tid == ref_tid) {
					LOG(TRACE) << " Found: EPC = " << parseUR.epc.epc << endl;
					if ( parseUR.epc.epc == epc ) {
						LOG(TRACE) << " Success: EPC = " << parseUR.epc.epc << endl;
						ret = RFID_OK;
						break;
					}
				}
			}
		}
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
		vector<RFID_EPC_STATISTICS> stat_result;
		DoStatisticHelper(reader_result, stat_result);

                // fill output buffer
		int i = 0;
		for (auto item : stat_result) {
			buffer[i++] = std::move(item);
			*stat_count = i;
			if ( i >= (int)stat_result.size() ) {
				ret = RFID_ERR_BUFFER_OVERFLOW;
				break;
			}
		}
	}
	return ret;
}


int RFSetLoopTime(HANDLE h, unsigned int looptime_ms)
{
	int ret;

        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		// SelectTag using bits as length
		ret = hm.get_rfid_ptr(h)->SetLoopTime( looptime_ms );
	}
	return ret;
}



int RFGetLoopTime(HANDLE h, unsigned int* looptime_ms)
{
	int ret;
        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		ret = hm.get_rfid_ptr(h)->GetLoopTime( *looptime_ms );
	}
	return ret;
}



int RFSetLoopAntenna(HANDLE h, uint32_t antennas)
{
	int ret;
        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	}
	ret = hm.get_rfid_ptr(h)->SetLoopAntenna( antennas );
	return ret;
}


int RFGetLoopAntenna(HANDLE h, unsigned int* antennas) {
	int ret;
        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	}
	ret = hm.get_rfid_ptr(h)->GetLoopAntenna( *antennas );
	return ret;
}



int RFSetPower(HANDLE h, int nPower)
{
	int pnResult;
	if ( !hm.is_valid_handle(h) ) {
		return RFID_ERR_INVALID_HANDLE;
	} else {
		return hm.get_rfid_ptr(h)->SetPower( nPower, &pnResult );
	}
}


int RFGetPower(HANDLE h, int* nPower)
{
	if ( !hm.is_valid_handle(h) ) {
		return RFID_ERR_INVALID_HANDLE;
	} else {
		return hm.get_rfid_ptr(h)->GetPower( *nPower );
	}
}


void RFClose(HANDLE h)
{
	hm.remove_handle_unit(h);
}
