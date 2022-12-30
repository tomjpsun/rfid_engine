#include <iostream>
#include <exception>
#include <chrono>
#include <thread>
#include <algorithm>
#include <chrono>
#include "aixlog.hpp"
#include "common.hpp"
#include "cmd_handler.hpp"
#include "cpp_if.hpp"
#include "conn_queue.hpp"
#include "rfid_config.hpp"
#include "rfid_if.hpp"
#include "parser.hpp"
#include "c_if.h"
#include "cruise_type.hpp"

#undef WAIT_RF_MODULE_TEST
#undef TEST_HEARTBEAT
#undef TEST_REBOOT
#undef TEST_WRITE
#define TEST_GPIO

using namespace std;
using namespace rfid;
using namespace cruise_namespace;

bool TEST_EN = false;

void thread_proc(ReaderSettings rs, int loop_count)
{
	int stat_array_size = 20;
	RFID_EPC_STATISTICS epc_stat_array[stat_array_size];


	HANDLE handle = RFOpen(rs);
	char* json_str;
	int json_len;

        if ( handle < 0 ) {
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "RFOpen(" << rs.ipv4 << ") failed, handle = " << handle << endl;
		return;
	} else {
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "open ReaderID: " << RFReaderID(handle) << endl;
	}
	while (loop_count-- > 0) {
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << ">>>  >>> loop_count = " << loop_count << endl;
		int power = 0;
		RFGetPower( handle, &power);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << " power: " << power << endl;
		if (power != rs.power)
			RFSetPower( handle, rs.power);

		uint32_t antenna;
		RFGetLoopAntenna( handle, &antenna );
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "RFGetLoopAntenna: previous 0x" << hex << antenna << endl;
		int r = RFSetLoopAntenna( handle, rs.antennas );
		if (r != RFID_OK ) {
			LOG(SEVERITY::TRACE) << COND(TEST_EN) << "RFSetLoopAntenna error, code = " << hex << r << endl;
			LOG(SEVERITY::TRACE) << COND(TEST_EN) << "Exit Test" << endl;
			break;
		}
		RFGetLoopAntenna( handle, &antenna );
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "RFGetLoopAntenna: currently 0x" << hex << antenna << endl;

		RFSetLoopTime( handle, rs.loop_time );
		unsigned int loopTime = 0;
		RFGetLoopTime( handle, &loopTime);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "RFSetLoopTime: " << loopTime << endl;

		RFSetSystemTime( handle );

		//RFInventoryEPC(handle, 3, false, &json_str, &json_len);
		//LOG(SEVERITY::TRACE) << COND(TEST_EN) << json_str << endl;
		//LOG(SEVERITY::TRACE) << COND(TEST_EN) << "[thread_proc]: total length: " << json_len << endl;

		RFReadMultiBank( handle, 3, true, RFID_MB_TID,
				 0, 6, &json_str, &json_len);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << json_str << endl;
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "handle: " << handle << ", [thread_prc]: total length: " << json_len << endl;

		RFStatistics( handle, 3, true, RFID_MB_TID,
			      0, 6, 0,    RF_STATISTICS_RULE_BY_EPC,
			      epc_stat_array, &stat_array_size);

		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "handle: " << handle << ", epc_stat_array size  = " << stat_array_size << endl;

		for (int i=0; i<stat_array_size; i++) {
			Cruise cruise{epc_stat_array[i]};
			cruise.convert_to_local_time();
			LOG(SEVERITY::TRACE) << COND(TEST_EN) << "handle: " << handle
			     << "[" << i << "].epc = " << cruise.epc << endl
			     << ", tid = " << cruise.tid << endl
			     << ", antenna = " << cruise.antenna << endl
			     << ", timestamp = " << cruise.timestamp << endl
			     << ", count = " << cruise.count << endl;
		}

		struct tm mytime;
		RFGetTime( handle, mytime );
		dumpDateTime(mytime);

		//RFSingleCommand( handle, (char *)"U3", 2, &json_str, &json_len );
		//LOG(SEVERITY::TRACE) << COND(TEST_EN) << json_str << endl;
		//LOG(SEVERITY::TRACE) << COND(TEST_EN) << "total length: " << json_len << endl;
#if TEST_WRITE
                char ref_tid[25];
		char new_epc[25];
		snprintf( ref_tid, 25, "%s", "E2801160200074CF085909AD");
		snprintf( new_epc, 25, "%s", "999988887777666655554444");
		int ret = RFWriteEPC( handle, ref_tid, 24,
				      new_epc, 24, true);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "handle: " << handle << ", RFWriteEPC() ret = " << ret << endl;
#endif
	}
	RFClose(handle);
}

int c_test(ReaderSettings* rs, int loop_count, bool thread_test = false)
{
	std::thread thread_func[2];

	thread_func[0] = std::thread(thread_proc, rs[0], loop_count);
	if (thread_test) {
		thread_func[1] = std::thread(thread_proc, rs[1], loop_count);
	}

	thread_func[0].join();
	if (thread_test)
		thread_func[1].join();

	return 0;
}



int cpp_test(ReaderSettings* rs, int loop_count)
{
	print_endian();
	RfidConfig cfg = RfidConfigFactory().get_config();

	RfidInterface rf(rs[0]);

	int ret;
	vector<string> read_mb;
	int err = 0;

	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ReaderID: " << string(rf.reader_settings.reader_id) << endl;

	RFID_READER_VERSION ver;
	ret = rf.GetVersion(ver);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "return: " << ret
	     << ", fw: " << ver.strFirmware
	     << ", hw: " << ver.strHardware
	     << ", id: " << ver.strReaderId
	     << ", band regulation: " << ver.strRfBandRegualation
	     << endl;

	std::string readerId;
	ret = rf.GetReaderID(readerId);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "GetReaderID return: " << ret << ", readerId: " << readerId << endl;

	uint32_t ant = 0;
	ret = rf.GetLoopAntenna(ant);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "GetLoopAntenna return: " << ret << ", antenna: " << ant <<  endl;

	unsigned int ant_test = 0x1111;
	unsigned int ant_after_test = 0;
	ret = rf.SetLoopAntenna(ant_test);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "SetLoopAntenna return: " << ret;
	ret = rf.GetLoopAntenna(ant_after_test);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << ", new antenna value = " << ant_after_test <<  endl;
	// restore antenna value before test
	ret = rf.SetLoopAntenna(ant);

#ifdef WAIT_RF_MODULE_TEST

	RFID_REGULATION regu;
	ret = rf.SetRegulation(REGULATION_US);
	ret = rf.GetRegulation(regu);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "return: " << ret
	     << ", regulation: " << int(regu)
	     << endl;

	int power;
	ret = rf.GetPower(power);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "return: " << ret
	     << ", power: " << power << endl;

	int pnResult;

	ret = rf.SetPower(20, &pnResult);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret: " << ret
	     << ", pnResult: " << pnResult
	     << endl;

	ret = rf.GetPower(power);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "return: " << ret
	     << ", new power: " << power << endl;

#endif // WAIR_RF_MODULE_TEST

        unsigned int antenna = 0;
	bool hub = false;
	ret = rf.GetSingleAntenna(antenna, hub);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret: " << ret
	     << ", antenna: " << antenna << ", hub: " << hub << endl;


	unsigned int loopAntenna = 0;
	ret = rf.GetLoopAntenna(loopAntenna);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret: " << ret
	     << ", loop antenna: " << loopAntenna << endl;

	unsigned int loopTime;
	ret = rf.GetLoopTime(loopTime);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret: " << ret
	     << ", loop time: " << loopTime << endl;

	struct tm time;
	ret = rf.GetTime(time);
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret: " << ret
	     << ", sec: "  << time.tm_sec
	     << ", min: "  << time.tm_min
	     << ", hour: " << time.tm_hour
	     << ", month day: "  << time.tm_mday
	     << ", month: "<< time.tm_mon + 1
	     << ", year: " << time.tm_year + 1900
	     << ", week day: " << time.tm_wday
	     << ", year day: " << time.tm_yday
	     << ", dst: " << time.tm_isdst
	     << endl;

	ret = rf.SetSystemTime();

	for ( int i = 0; i < loop_count; i++) {

		// test InventoryEPC w/o loop
		ret = rf.InventoryEPC(3, false, read_mb);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret:" << ret
		     << ", Inventory w/o loop:" << endl;
		for (auto iter : read_mb)
			LOG(SEVERITY::TRACE) << COND(TEST_EN) << iter << endl;
		read_mb.clear();

		// test InventoryEPC with loop
		ret  = rf.InventoryEPC(3, true, read_mb);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret:" << ret
		     << ", Inventory with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseU parseU(iter);
			LOG(SEVERITY::TRACE) << COND(TEST_EN) << parseU << endl;
		}
		read_mb.clear();

		// test loop ReadMultiBank()
		ret = rf.ReadMultiBank(3, true, RFID_MB_TID, 0, 6, read_mb, err);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret:" << ret << ", err: " << err
		     << ", Read Multi Bank TID with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseUR parseUR(iter, RFID_MB_TID);
			LOG(SEVERITY::TRACE) << COND(TEST_EN) << parseUR << endl;
		}
		read_mb.clear();

		// test non-loop ReadMultiBank()
		err = 0;
		ret = rf.ReadMultiBank(3, false, RFID_MB_TID, 0, 6, read_mb, err);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret:" << ret << ", err: " << err
		     << ", Read Multi Bank TID w/o loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseUR parseUR(iter, RFID_MB_TID);
			LOG(SEVERITY::TRACE) << COND(TEST_EN) <<  parseUR << endl;
		}
		read_mb.clear();

		// test non-loop ReadBank()
		ret = rf.ReadBank( false, RFID_MB_TID, 0, 6, read_mb);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret:" << ret
		     << ", Read Bank TID w/o loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseR parseR(iter);
			LOG(SEVERITY::TRACE) << COND(TEST_EN) <<  parseR << endl;
		}
		read_mb.clear();

		// test loop ReadBank()
		ret = rf.ReadBank( true, RFID_MB_TID, 0, 6, read_mb);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret:" << ret
		     << ", Read Bank TID with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseR parseR(iter);
			LOG(SEVERITY::TRACE) << COND(TEST_EN) << parseR << endl;
		}
		read_mb.clear();

		// test loop ReadBank()
		ret = rf.ReadBank( true, RFID_MB_EPC, 0, 6, read_mb);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "ret:" << ret
		     << ", Read Bank EPC with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseR parseR(iter);
			LOG(SEVERITY::TRACE) << COND(TEST_EN) << parseR << endl;
		}
		read_mb.clear();

		// test loop ReadBank()
		ret = rf.ReadBank( true, RFID_MB_TID, 0, 4, read_mb);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "result:" << ret
		     << ", Read Bank TID with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseR parseR(iter);
			LOG(SEVERITY::TRACE) << COND(TEST_EN) << parseR << endl;
		}
	} // end for loop

#ifdef TEST_HEARTBEAT
	char user_data[100];
	snprintf(user_data, 100, "%s", "this is test user data\n");

	HeartBeatCallackFunc heartbeat = [](string reader_id, void* user) {
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "application: reader_id = " << reader_id << endl;
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << (char*)user;
		return false;
	};

	rf.OpenHeartbeat(3000, heartbeat, user_data);

	for (int i=0; i<5; i++) {
		std::this_thread::sleep_for(1000ms);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "sleep loop count: " << i << endl;
	}

	rf.CloseHeartbeat();

	for (int i=0; i<1; i++) {
		std::this_thread::sleep_for(1000ms);
		LOG(SEVERITY::TRACE) << COND(TEST_EN) << "sleep loop count: " << i << endl;
	}
#endif // TEST_HEARTBEAT

#ifdef TEST_REBOOT
        LOG(SEVERITY::TRACE) << COND(TEST_EN) << "start test Reboot()" << flush << endl;
	rf.Reboot();
        LOG(SEVERITY::TRACE) << COND(TEST_EN) << "end test Reboot()" << flush << endl;
#endif

#ifdef TEST_WRITE

        rf.SelectTag( RFID_MB_TID, 0x0, 0x60, std::string{"E2801160200074CF085909AD"});
	std::string pass;
	rf.Password(pass);
	rf.WriteBank( RFID_MB_EPC, 2, 4, std::string{"9999888877776666"} );
#endif

/* previous version
#ifdef TEST_GPIO
	uint8_t bits = 0;
	rf.SetGPO(1, true);
	rf.SetGPO(2, true);
	rf.GetGPO( bits );
	rf.SetGPO(2, false);
	rf.GetGPO( bits );
        std::this_thread::sleep_for(400ms);
	rf.SetGPO(1, false);
#endif
*/

// refactor version, by default "COM_x" is shared to NO/NC port
#ifdef TEST_GPIO
        uint8_t bits = 0;
	rf.SetGPO(1, true); // set "Normal Open" true for circuit connected
	rf.GetGPO( bits );
        std::this_thread::sleep_for(400ms);
	rf.SetGPO(1, false); // set "Normal Open" false for circuit dis-connected
#endif
        return 0;
}


int main(int argc, char** argv)
{
	int ret;
	ReaderSettings rs[2] = {
		{
		.type = ReaderSettingsConnectionType(SOCKET), // socket type
		.ipv4 =	{192, 168, 45, 91 }, // ipv4
		.ipv6 =	{}, // ipv6
		.dev_name = {}, // dev name
		.port = 1001, // port
		.power = 28, // power
		.loop_time = 40, // loop time
		.antennas = RF_HUB_1_ANTENNA_1, // antenna id
		.level = 1
		},
		{
		.type = ReaderSettingsConnectionType(SOCKET), // socket type
		.ipv4 = {192, 168, 88, 96 }, // ipv4
		.ipv6 = {}, // ipv6
		.dev_name = {}, // dev name
		.port = 1001, // port
		.power = 28, // power
		.loop_time = 40, // loop time
		.antennas = RF_HUB_1_ANTENNA_1, // antenna id
		.level = 1
		}
	};

        if ( (ret = RFModuleInit()) != RFID_OK ) {
		return ret;
	}
	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "start c_test() \n";
	time_t t = time(NULL);
	printf("local time:     %s", asctime(localtime(&t)));
	int loop_count = 3;
	int rs_index = stoi(argv[1]);

	//bool enable_thread = false;
	//return c_test(&rs[rs_index], loop_count, enable_thread);

	LOG(SEVERITY::TRACE) << COND(TEST_EN) << "start cpp_test " << " index = " << rs_index << endl;

        return cpp_test(&rs[rs_index], loop_count);

}
