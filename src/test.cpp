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
#include "rfid_if.hpp"
#include "parser.hpp"

using namespace std;
using namespace rfid;


void thread_proc(int index, int loop_count)
{
	HANDLE handle = RFOpen(index);
	char* json_str;
	int json_len;

        if ( handle < 0 ) {
		cout << "RFOpen(" << index << ") failed, handle = " << handle << endl;
		return;
	}
	while (loop_count > 0) {
		RFInventoryEPC(handle, 3, false, &json_str, &json_len);
		//cout << json_str << endl;
		cout << "[thread_proc]: total length: " << json_len << endl;

		RFReadMultiBank( handle, 3, true, RFID_MB_TID,
				 0, 6, &json_str, &json_len);
		//cout << json_str << endl;
		cout << "[thread_prc]: total length: " << json_len << endl;
	}
	RFClose(handle);
}

const bool thread_test = true;

int c_test()
{
	std::thread thread_func;
	int loop_count = 150;

	if (thread_test) {
		thread_func = std::thread(thread_proc, 1, loop_count);
	}

	HANDLE handle = RFOpen(0);
	if ( handle < 0 ) {
		cout << "RFOpen() failed, handle = " << handle << endl;
		return handle;
	}
        char* json_str;
	int json_len;

        RFInventoryEPC(handle, 3, false, &json_str, &json_len);
	cout << json_str << endl;
	cout << "total length: " << json_len << endl;

	RFReadMultiBank( handle, 3, true, RFID_MB_TID,
			 0, 6, &json_str, &json_len);
	cout << json_str << endl;
	cout << "total length: " << json_len << endl;

	//RFSingleCommand( handle, (char *)"U3", 2, &json_str, &json_len );
	//cout << json_str << endl;
	//cout << "total length: " << json_len << endl;

	if (thread_test)
		thread_func.join();

        RFClose(handle);
	return 0;
}




int cpp_test()
{
        PQParams pq_params = {
		.ip_type = IP_TYPE_IPV4, // IP_TYPE_IPV(4|6)
		.port = 1001 // default 1001
	};
	sprintf(pq_params.ip_addr, "192.168.88.91");
	print_endian();
	RfidInterface rf(pq_params);

	int ret;
	vector<string> read_mb;
	int err = 0;
#if 0
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
	cout << "return: " << ret << ", readerId: " << readerId << endl;


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
#endif
	rf.SelectTag( RFID_MB_TID, 0x0, 0x60, std::string{"E28011602000603F085309AD"} );
	//rf.SelectTag( RFID_MB_EPC, 0x20, 0x50, std::string{"99998888777766665555"} );
	std::string pass;
	rf.Password(pass);
	rf.WriteBank( RFID_MB_EPC, 2, 4, std::string{"9999888877776666"} );

	ret = rf.ReadMultiBank(3, true, RFID_MB_TID, 0, 6, read_mb, err);
        cout << "ret:" << ret << ", err: " << err
	     << ", Read Multi Bank TID with loop:" << endl;
        for (auto iter : read_mb) {
		RfidParseUR parseUR(iter, RFID_MB_TID);
		nlohmann::json j = parseUR;
		cout << j << endl;
	}
	read_mb.clear();


        return 0;
}


int main(int argc, char** argv)
{
	return cpp_test();
}
