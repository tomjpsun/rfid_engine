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

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
	char ** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
	{
		return *itr;
	}
	return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
	return std::find(begin, end, option) != end;
}

void print_usage_hint()
{
	cout << "Usage : -l 1000" << endl;
	cout << "\t -l: read loop count, set -1 for infinite loop, default = 1000" << endl;
	cout << "\t finish program on condition is met" << endl;
	cout << "\t w/o options, show this hint" << endl;
}

int main(int argc, char** argv)
{
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug);
        PQParams pq_params = {
		.ip_type = IP_TYPE_IPV4, // IP_TYPE_IPV(4|6)
		.port = 1001 // default 1001
	};
	sprintf(pq_params.ip_addr, "192.168.88.91");
	print_endian();
	RfidInterface rf;
	bool result;

	RFID_READER_VERSION ver;
	rf.GetVersion(ver);
	cout << "fw: " << ver.strFirmware
	     << ", hw: " << ver.strHardware
	     << ", id: " << ver.strReaderId
	     << ", band regulation: " << ver.strRfBandRegualation
	     << endl;

	std::string readerId;
	result = rf.GetReaderID(readerId);
	cout << "result: " << result << ", readerId: " << readerId << endl;

#if 0
	RFID_REGULATION regu;
	rf.SetRegulation(REGULATION_US);
	rf.GetRegulation(regu);
	cout << "regulation: " << int(regu) << endl;

	int power;
	rf.GetPower(power);
	cout << "power: " << power << endl;

	int pnResult;
	// 10 dbm
	result = rf.SetPower(2, &pnResult);
	cout << "result: " << result
	     << ", pnResult: " << pnResult
	     << endl;

	rf.GetPower(power);
	cout << "new power: " << power << endl;
#endif
	unsigned int antenna = 0;
	bool hub = false;
	rf.GetSingleAntenna(antenna, hub);
	cout << "antenna: " << antenna << ", hub: " << hub << endl;

	unsigned int loopAntenna = 0;
	rf.GetLoopAntenna(loopAntenna);
	cout << "loop antenna: " << loopAntenna << endl;

	unsigned int loopTime;
	rf.GetLoopTime(loopTime);
	cout << "loop time: " << loopTime << endl;

	struct tm time;
	result = rf.GetTime(time);
	cout << "result: " << result
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
	vector<string> inventory_result;
	result = rf.InventoryEPC(3, false, inventory_result);
	cout << "Inventory w/o loop:" << endl;
	for (auto iter : inventory_result)
		cout << iter << endl;

	inventory_result.clear();

	// test InventoryEPC with loop
	result  = rf.InventoryEPC(3, true, inventory_result);
        cout << "result:" << result
	     << ", Inventory with loop:" << endl;
        for (auto iter : inventory_result) {
		RfidParseU parseU(iter);
		cout << parseU << endl;
	}

	// test loop ReadMultiBank()
	vector<string> read_mb;
	int err = 0;
	result = rf.ReadMultiBank(3, true, RFID_MB_TID, 0, 6, read_mb, err);
        cout << "result:" << result << ", err: " << err
	     << ", Read Multi Bank TID with loop:" << endl;
        for (auto iter : read_mb) {
		RfidParseUR parseUR(iter, RFID_MB_TID);
		cout << parseUR << endl;
	}
	read_mb.clear();

	// test non-loop ReadMultiBank()
	err = 0;
	result = rf.ReadMultiBank(3, false, RFID_MB_TID, 0, 6, read_mb, err);
        cout << "result:" << result << ", err: " << err
	     << ", Read Multi Bank TID w/o loop:" << endl;
        for (auto iter : read_mb) {
		RfidParseUR parseUR(iter, RFID_MB_TID);
		cout <<  parseUR << endl;
	}
	read_mb.clear();


	// test non-loop ReadBank()
	result = rf.ReadBank( false, RFID_MB_TID, 0, 6,
			      read_mb);
	cout << "result:" << result
	     << ", Read Bank TID w/o loop:" << endl;
        for (auto iter : read_mb) {
		RfidParseR parseR(iter);
		cout <<  parseR << endl;
	}
	read_mb.clear();

	// test loop ReadBank()
	result = rf.ReadBank( true, RFID_MB_TID, 0, 6,
			      read_mb);
	cout << "result:" << result
	     << ", Read Bank TID with loop:" << endl;
        for (auto iter : read_mb) {
		RfidParseR parseR(iter);
		cout << parseR << endl;
	}
	read_mb.clear();

	// test loop ReadBank()
	result = rf.ReadBank( true, RFID_MB_EPC, 0, 6,
			      read_mb);
	cout << "result:" << result
	     << ", Read Bank EPC with loop:" << endl;
        for (auto iter : read_mb) {
		RfidParseR parseR(iter);
		cout << parseR << endl;
	}

	read_mb.clear();

        // test loop ReadBank()
	result = rf.ReadBank( true, RFID_MB_TID, 0, 4,
			      read_mb);
	cout << "result:" << result
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

	rf.WriteBank( RFID_MB_EPC, 2, 4, std::string{"1111222233334444"} );
	cout << "exit" << endl;
	return 0;
}
