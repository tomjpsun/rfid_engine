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


#undef WAIT_RF_MODULE_TEST
#undef TEST_HEARTBEAT
#undef TEST_REBOOT
#undef TEST_WRITE

using namespace std;
using namespace rfid;

extern RfidConfig g_cfg;

void thread_proc(ReaderSettings rs, int loop_count)
{
	int stat_array_size = 20;
	RFID_EPC_STATISTICS epc_stat_array[stat_array_size];


	HANDLE handle = RFOpen(rs);
	char* json_str;
	int json_len;

        if ( handle < 0 ) {
		cout << "RFOpen(" << rs.ipv4 << ") failed, handle = " << handle << endl;
		return;
	}
	while (loop_count-- > 0) {
		uint32_t antenna;
		RFGetLoopAntenna( handle, &antenna );
		cout << "RFGetLoopAntenna: previous 0x" << hex << antenna << endl;
		int r = RFSetLoopAntenna( handle, RF_HUB_1_ANTENNA_1 | RF_HUB_1_ANTENNA_4 );
		if (r != RFID_OK ) {
			cout << "RFSetLoopAntenna error, code = " << hex << r << endl;
			cout << "Exit Test" << endl;
			break;
		}
		RFGetLoopAntenna( handle, &antenna );
		cout << "RFGetLoopAntenna: currently 0x" << hex << antenna << endl;
		unsigned int loopTime = 200;
		RFSetLoopTime( handle, loopTime );
		cout << "RFSetLoopTime: " << loopTime << endl;

		//RFInventoryEPC(handle, 3, false, &json_str, &json_len);
		//cout << json_str << endl;
		//cout << "[thread_proc]: total length: " << json_len << endl;

		RFReadMultiBank( handle, 3, true, RFID_MB_TID,
				 0, 6, &json_str, &json_len);
		cout << json_str << endl;
		cout << "handle: " << handle << ", [thread_prc]: total length: " << json_len << endl;

		RFStatistics( handle, 3, true, RFID_MB_TID,
			      0, 6, 1000,    RF_STATISTICS_RULE_BY_EPC,
			      epc_stat_array, &stat_array_size);

		cout << "handle: " << handle << ", epc_stat_array size  = " << stat_array_size << endl;

		for (int i=0; i<stat_array_size; i++)
			cout << "handle: " << handle
			     << ", epc_stat_array[" << i << "].epc = " << epc_stat_array[i].epc << endl
			     << ", tid = " << epc_stat_array[i].tid << endl
			     << ", antenna = " << epc_stat_array[i].antenna << endl
			     << ", count = " << epc_stat_array[i].count << endl;

		struct tm mytime;
		RFGetTime( handle, mytime );
		dumpDateTime(mytime);

		//RFSingleCommand( handle, (char *)"U3", 2, &json_str, &json_len );
		//cout << json_str << endl;
		//cout << "total length: " << json_len << endl;
#if TEST_WRITE
                char ref_tid[25];
		char new_epc[25];
		snprintf( ref_tid, 25, "%s", "E2801160200074CF085909AD");
		snprintf( new_epc, 25, "%s", "999988887777666655554444");
		int ret = RFWriteEPC( handle, ref_tid, 24,
				      new_epc, 24, true);
		cout << "handle: " << handle << ", RFWriteEPC() ret = " << ret << endl;
#endif
	}
	RFClose(handle);
}

int c_test(ReaderSettings* rs, bool thread_test = false)
{
	std::thread thread_func[2];
	int loop_count = 5;

	thread_func[0] = std::thread(thread_proc, rs[0], loop_count);
	if (thread_test) {
		thread_func[1] = std::thread(thread_proc, rs[1], loop_count);
	}

	thread_func[0].join();
	if (thread_test)
		thread_func[1].join();

	return 0;
}



int cpp_test(ReaderSettings* rs)
{
	int loop_count = 1;

	print_endian();
	RfidConfig cfg = RfidConfigFactory().get_config();

	RfidInterface rf(rs[0]);

	int ret;
	vector<string> read_mb;
	int err = 0;

	RFID_READER_VERSION ver;
	ret = rf.GetVersion(ver);
	cout << "return: " << ret
	     << ", fw: " << ver.strFirmware
	     << ", hw: " << ver.strHardware
	     << ", id: " << ver.strReaderId
	     << ", band regulation: " << ver.strRfBandRegualation
	     << endl;

	std::string readerId;
	ret = rf.GetReaderID(readerId);
	cout << "GetReaderID return: " << ret << ", readerId: " << readerId << endl;

	uint32_t ant = 0;
	ret = rf.GetLoopAntenna(ant);
	cout << "GetLoopAntenna return: " << ret << ", antenna: " << ant <<  endl;

	unsigned int ant_test = 0x1111;
	unsigned int ant_after_test = 0;
	ret = rf.SetLoopAntenna(ant_test);
	cout << "SetLoopAntenna return: " << ret;
	ret = rf.GetLoopAntenna(ant_after_test);
	cout << ", new antenna value = " << ant_after_test <<  endl;
	// restore antenna value before test
	ret = rf.SetLoopAntenna(ant);

#ifdef WAIT_RF_MODULE_TEST

	RFID_REGULATION regu;
	ret = rf.SetRegulation(REGULATION_US);
	ret = rf.GetRegulation(regu);
	cout << "return: " << ret
	     << ", regulation: " << int(regu)
	     << endl;

	int power;
	ret = rf.GetPower(power);
	cout << "return: " << ret
	     << ", power: " << power << endl;

	int pnResult;

	ret = rf.SetPower(20, &pnResult);
	cout << "ret: " << ret
	     << ", pnResult: " << pnResult
	     << endl;

	ret = rf.GetPower(power);
	cout << "return: " << ret
	     << ", new power: " << power << endl;

#endif // WAIR_RF_MODULE_TEST

        unsigned int antenna = 0;
	bool hub = false;
	ret = rf.GetSingleAntenna(antenna, hub);
	cout << "ret: " << ret
	     << ", antenna: " << antenna << ", hub: " << hub << endl;


	unsigned int loopAntenna = 0;
	ret = rf.GetLoopAntenna(loopAntenna);
	cout << "ret: " << ret
	     << ", loop antenna: " << loopAntenna << endl;

	unsigned int loopTime;
	ret = rf.GetLoopTime(loopTime);
	cout << "ret: " << ret
	     << ", loop time: " << loopTime << endl;

	struct tm time;
	ret = rf.GetTime(time);
	cout << "ret: " << ret
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

	for ( int i = 0; i < loop_count; i++) {

		// test InventoryEPC w/o loop
		ret = rf.InventoryEPC(3, false, read_mb);
		cout << "ret:" << ret
		     << ", Inventory w/o loop:" << endl;
		for (auto iter : read_mb)
			cout << iter << endl;
		read_mb.clear();

		// test InventoryEPC with loop
		ret  = rf.InventoryEPC(3, true, read_mb);
		cout << "ret:" << ret
		     << ", Inventory with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseU parseU(iter);
			cout << parseU << endl;
		}
		read_mb.clear();

		// test loop ReadMultiBank()
		ret = rf.ReadMultiBank(3, true, RFID_MB_TID, 0, 6, read_mb, err);
		cout << "ret:" << ret << ", err: " << err
		     << ", Read Multi Bank TID with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseUR parseUR(iter, RFID_MB_TID);
			cout << parseUR << endl;
		}
		read_mb.clear();

		// test non-loop ReadMultiBank()
		err = 0;
		ret = rf.ReadMultiBank(3, false, RFID_MB_TID, 0, 6, read_mb, err);
		cout << "ret:" << ret << ", err: " << err
		     << ", Read Multi Bank TID w/o loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseUR parseUR(iter, RFID_MB_TID);
			cout <<  parseUR << endl;
		}
		read_mb.clear();

		// test non-loop ReadBank()
		ret = rf.ReadBank( false, RFID_MB_TID, 0, 6, read_mb);
		cout << "ret:" << ret
		     << ", Read Bank TID w/o loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseR parseR(iter);
			cout <<  parseR << endl;
		}
		read_mb.clear();

		// test loop ReadBank()
		ret = rf.ReadBank( true, RFID_MB_TID, 0, 6, read_mb);
		cout << "ret:" << ret
		     << ", Read Bank TID with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseR parseR(iter);
			cout << parseR << endl;
		}
		read_mb.clear();

		// test loop ReadBank()
		ret = rf.ReadBank( true, RFID_MB_EPC, 0, 6, read_mb);
		cout << "ret:" << ret
		     << ", Read Bank EPC with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseR parseR(iter);
			cout << parseR << endl;
		}
		read_mb.clear();

		// test loop ReadBank()
		ret = rf.ReadBank( true, RFID_MB_TID, 0, 4, read_mb);
		cout << "result:" << ret
		     << ", Read Bank TID with loop:" << endl;
		for (auto iter : read_mb) {
			RfidParseR parseR(iter);
			cout << parseR << endl;
		}
	} // end for loop

#ifdef TEST_HEARTBEAT
	char user_data[100];
	snprintf(user_data, 100, "%s", "this is test user data\n");

	HeartBeatCallackFunc heartbeat = [](string reader_id, void* user) {
		cout << "application: reader_id = " << reader_id << endl;
		cout << (char*)user;
		return false;
	};

	rf.OpenHeartbeat(3000, heartbeat, user_data);

	for (int i=0; i<5; i++) {
		std::this_thread::sleep_for(1000ms);
		cout << "sleep loop count: " << i << endl;
	}

	rf.CloseHeartbeat();

	for (int i=0; i<1; i++) {
		std::this_thread::sleep_for(1000ms);
		cout << "sleep loop count: " << i << endl;
	}
#endif // TEST_HEARTBEAT

#ifdef TEST_REBOOT
        cout << "start test Reboot()" << flush << endl;
	rf.Reboot();
        cout << "end test Reboot()" << flush << endl;
#endif

#ifdef TEST_WRITE

        rf.SelectTag( RFID_MB_TID, 0x0, 0x60, std::string{"E2801160200074CF085909AD"});
	std::string pass;
	rf.Password(pass);
	rf.WriteBank( RFID_MB_EPC, 2, 4, std::string{"9999888877776666"} );
#endif

	ret = rf.ReadMultiBank(3, true, RFID_MB_TID, 0, 6, read_mb, err);
	cout << "ret:" << ret << ", err: " << err
	     << ", Read Multi Bank TID with loop:" << endl;
	for (auto iter : read_mb) {
		RfidParseUR parseUR(iter, RFID_MB_TID);
		nlohmann::json j = parseUR;
		cout << j << endl;
	}
	read_mb.clear();

	uint8_t bits = 0;
	rf.SetGPO(1, true);
	rf.SetGPO(2, true);
	rf.GetGPO( bits );
	rf.SetGPO(2, false);
	rf.GetGPO( bits );
        std::this_thread::sleep_for(1000ms);
        rf.SetGPO(1, false);

        return 0;
}


int main(int argc, char** argv)
{
	int ret;
	ReaderSettings rs[2] = {
		{
			ReaderSettingsType(SOCKET), // socket type
			{192, 168, 88, 94 }, // ipv4
			{}, // ipv6
			{}, // dev name
			1001, // port
			28, // power
			40, // loop time
			1 // antenna id
		},
		{
			ReaderSettingsType(SOCKET), // socket type
			{192, 168, 88, 91 }, // ipv4
			{}, // ipv6
			{}, // dev name
			1001, // port
			28, // power
			40, // loop time
			1 // antenna id
		}
	};

        if ( (ret = RFModuleInit()) != RFID_OK ) {
		return ret;
	}
	cout << "start c_test() \n";
	return c_test(rs, true);

	//cout << "start cpp_test \n";
        //int device_index = 0;
        //return cpp_test(rs);

}
