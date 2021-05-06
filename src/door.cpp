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
	int ret;
	vector<string> read_mb;
	int err = 0;

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


	// test read MultiBank()
        result = rf.ReadMultiBank(3, true, RFID_MB_TID, 0, 6, read_mb, err);
        cout << "result:" << result << ", err: " << err
	     << ", Read Multi Bank TID with loop:" << endl;
        for (auto iter : read_mb) {
		RfidParseUR parseUR(iter, RFID_MB_TID);
		cout << parseUR << endl;
	}
	read_mb.clear();

        return 0;
}
