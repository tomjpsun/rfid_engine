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
#include <mutex>
#include <map>
#include <initializer_list>
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
#include "curl_stub.h"
#include "ulog_type.h"

using namespace std;
using namespace rfid;
using json = nlohmann::json;
using namespace std::chrono;
using namespace ulog_namespace;

// get value from config
bool DBG_EN;

// holds obj which user had opened, erase on user close it
static HandleManager hm{};


static vector<AixLog::Severity> LogLevelMap = {
    AixLog::Severity::trace,
    AixLog::Severity::debug,
    AixLog::Severity::info,
    AixLog::Severity::notice,
    AixLog::Severity::warning,
    AixLog::Severity::error,
    AixLog::Severity::fatal
};


std::once_flag onceFlag;

// init all to 0
RfidConfig g_cfg{};


// ========== Helper functions ==========

void dumpDateTime(const struct tm& dTime)
{
	LOG(SEVERITY::INFO)
		<< " yyyy/mm/dd hh:mm:ss  "
		<< dTime.tm_year + 1900 << "/"
		<< dTime.tm_mon + 1 << "/"
		<< dTime.tm_mday << " "
		<< dTime.tm_hour << ":"
		<< dTime.tm_min << ":"
		<< dTime.tm_sec ;
}


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


void DoStatisticHelper(HANDLE h,
		       vector<RfidParseUR> &reader_result,
		       vector<RFID_EPC_STATISTICS>& stat_result,
		       RF_STATISTICS_RULE statistics_rule)
{
	// from reader_result, collect count on each EPC,
	// and write to stat_result

	for (RfidParseUR& parse : reader_result) {
		if (parse.has_data && parse.epc.is_match) {
			string epc = parse.epc.epc;
			string tid = parse.tid;
			RfidTime time = parse.time;
			int antenna = std::stoi( parse.antenna );
			vector<RFID_EPC_STATISTICS>::iterator p =
				std::find_if(stat_result.begin(),
					     stat_result.end(),
					     [=] (vector<RFID_EPC_STATISTICS>::value_type& item) {
						     if (item.antenna != antenna)
							     return false;
						     else {
							     // EPC alwayse exist, TID may be empty
							     if (statistics_rule == RF_STATISTICS_RULE_BY_EPC) {
								     return (item.epc == epc);

							     } else if ((statistics_rule == RF_STATISTICS_RULE_BY_TID) &&
									!tid.empty()) {
								     return (item.tid == tid);

							     } else if (statistics_rule == RF_STATISTICS_RULE_BY_EPC_OR_TID) {
								     return (item.epc == epc) || (item.tid == tid);

							     } else
								     return false;
						     }});

			// Not found in history: this is a new tag record

			if ( p == stat_result.end() ) {
				// init empty struct
				RFID_EPC_STATISTICS new_stat{};
				std::memcpy(new_stat.epc, epc.c_str(), EPC_LEN);
				std::memcpy(new_stat.tid, parse.tid.c_str(), TID_LEN);
                                string reader_id = hm.get_rfid_ptr(h)->reader_info.reader_id;
				std::memcpy(new_stat.readerID, reader_id.c_str(), READER_ID_LEN);
				new_stat.antenna = antenna;
				new_stat.count = 1;
				new_stat.year = time.year;
				new_stat.month = time.month;
				new_stat.day = time.day;
				new_stat.hour = time.hour;
				new_stat.min = time.min;
				new_stat.sec = time.sec;
				new_stat.ms = time.ms;
				stat_result.push_back(new_stat);
				LOG(SEVERITY::TRACE) << COND(DBG_EN) << "copied TID = " << new_stat.tid << endl;
			}
			else {
				p->count += 1;
				// since epc alwayse exist, fill missing tid, in case its empty
				string tmp_tid{p->tid};
				if (tmp_tid.empty() && !tid.empty())
					std::memcpy(p->tid, tid.c_str(), TID_LEN);
			}
		}
	}
}




void
sink_ulog_fn(const AixLog::Metadata& metadata, const std::string& message)
{
	CurlStub<Ulog> curl_stub {
		g_cfg.ulog_server_ip,
		g_cfg.ulog_server_port,
		"/ulog/add"};
	string logger_id = RfidConfigFactory().get_machine_id();
	string ts_message = "[" + metadata.timestamp.to_string() + "] " + message;
	Ulog ulog{logger_id, int(metadata.severity), ts_message};
	curl_stub.post(ulog);
#if 0
	{ // example of metadata
        cout << "Callback:\n\tmsg:   " << message
	     << "\n\ttag:   "          << metadata.tag.text
	     << "\n\tseverity: "          << AixLog::to_string(metadata.severity)
	     << " (" << (int)metadata.severity << ")\n\ttype:  ";

        if (metadata.timestamp)
		cout << "\ttime:  " << metadata.timestamp.to_string() << "\n";
        if (metadata.function)
		cout << "\tfunc:  "
		     << metadata.function.name
		     << "\n\tline:  "
		     << metadata.function.line
		     << "\n\tfile:  "
		     << metadata.function.file << "\n";
	}
#endif

}

// ========== API functions ==========

int RFModuleInit()
{
	int result = RFID_OK;
	g_cfg = RfidConfigFactory().get_config();
	DBG_EN = g_cfg.dbg_en;

	std::call_once( onceFlag, [] {
		std::vector<AixLog::log_sink_ptr> sink_vec;
		auto sink_cout = make_shared<AixLog::SinkCout>( LogLevelMap[g_cfg.log_level] );
		auto sink_file = make_shared<AixLog::SinkFile>( LogLevelMap[g_cfg.log_level], g_cfg.log_file);
		auto sink_system = make_shared<AixLog::SinkNative>("rfidengine", AixLog::Severity::trace);
		auto sink_ulog = make_shared<AixLog::SinkCallback>(AixLog::Severity::trace, sink_ulog_fn);
		if (g_cfg.en_log_cout)
			sink_vec.push_back(sink_cout);
		if (g_cfg.en_log_file)
			sink_vec.push_back(sink_file);
		if (g_cfg.en_log_syslog)
			sink_vec.push_back(sink_system);
		if (g_cfg.en_log_ulog)
			sink_vec.push_back(sink_ulog);

		AixLog::Log::init( sink_vec );
	});

	LOG(SEVERITY::NOTICE) << LOG_TAG
			      << "version :"
			      << MAJOR << "."
			      << MINOR << "."
			      << SUB_MINOR << endl;
	return result;
}


HANDLE RFOpen(int index)
{
	ReaderInfo info = g_cfg.reader_info_list[index];
	LOG(SEVERITY::DEBUG) << LOG_TAG
			     << "index = " << index << ", ip = " << info.settings[0] << endl;
	try {
		shared_ptr<RfidInterface> prf =
			shared_ptr<RfidInterface>(new RfidInterface(info));
		int r = hm.add_handle_unit(prf);
		LOG(SEVERITY::DEBUG) << LOG_TAG << "return handle = " << r << endl;
		return r;
	} catch (const std::exception& e) {
		std::cout << e.what() << '\n';
		return HANDLE{-1};
	}
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
		LOG(SEVERITY::TRACE) << COND(DBG_EN) << "json str = " << *json_str << endl;
	}
        return ret;
}


void RFSingleCommand(HANDLE h, char* userCmd, int userCmdLen, char **response_str, int* response_len)
{
	string response;

        if ( !hm.is_valid_handle(h) ) {
		LOG(SEVERITY::ERROR) << COND(DBG_EN) << " RFID_ERR_INVALID_HANDLE " << endl;
	} else {
		string user(userCmd, userCmdLen);
		response = hm.get_rfid_ptr(h)->SingleCommand(user);
		hm.clear_buffer(h);
		hm.append_data(h, response);
		*response_str = hm.get_data(h, response_len);
		LOG(SEVERITY::TRACE) << COND(DBG_EN) << "response = " << *response_str << endl;
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
		LOG(SEVERITY::TRACE) << COND(DBG_EN) << "json str = " << *json_str << endl;
	}
	return ret;
}

int RFGetTime(HANDLE h, struct tm& time)
{
	int ret = RFID_OK;
	if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		ret = hm.get_rfid_ptr(h)->GetTime(time);
	}
	return ret;
}


int RFSetTime(HANDLE h, struct tm time)
{
	int ret = RFID_OK;
	if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		ret = hm.get_rfid_ptr(h)->SetTime(time);
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
					LOG(TRACE) << COND(DBG_EN) << " Found: EPC = " << parseUR.epc.epc << endl;
					if ( parseUR.epc.epc == epc ) {
						LOG(TRACE) << COND(DBG_EN) << " Success: EPC = " << parseUR.epc.epc << endl;
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
		 RF_STATISTICS_RULE statistics_rule,
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
			LOG(SEVERITY::DEBUG) << COND(DBG_EN) << "elapsed time: " << du.count() << " ms" << endl;
			if (du.count() > reference_time)
				break;
		};
		vector<RFID_EPC_STATISTICS> stat_result;
		DoStatisticHelper(h, reader_result, stat_result, statistics_rule);

                // fill output buffer
		int i = 0;
		for (auto item : stat_result) {
			if ( i >= (int)stat_result.size() ) {
				ret = RFID_ERR_BUFFER_OVERFLOW;
				break;
			}
			buffer[i++] = std::move(item);
		}
		*stat_count = i;
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



int RFSetLoopAntenna(HANDLE h, unsigned int antennas)
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
