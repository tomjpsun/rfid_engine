#include <iostream>
#include <exception>
#include <chrono>
#include <thread>
#include <algorithm>
#include <chrono>
#include <string>
#include "aixlog.hpp"
#include "common.hpp"
#include "cmd_handler.hpp"
#include "cpp_if.hpp"
#include "conn_queue.hpp"
#include "rfid_if.hpp"
#include "parser.hpp"

using namespace std;
using namespace rfid;

class CmdOptions
{
public:
	int power;
	int looptime;
	string save_file;
};

static CmdOptions cmdOpt{};

void unused_commands()
{
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug);
        PQParams pq_params = {
		.ip_type = IP_TYPE_IPV4, // IP_TYPE_IPV(4|6)
		.port = 1001 // default 1001
	};
	sprintf(pq_params.ip_addr, "192.168.88.91");
	print_endian();
	RfidInterface rf;

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

	rf.SelectTag( RFID_MB_EPC, 0x20, 0x50, std::string{"99998888777766665555"} );
	std::string pass;
	rf.Password(pass);
	rf.WriteBank( RFID_MB_EPC, 2, 4, std::string{"9999888877776666"} );

	ret = rf.ReadMultiBank(3, true, RFID_MB_TID, 0, 6, read_mb, err);
        cout << "ret:" << ret << ", err: " << err
	     << ", Read Multi Bank TID with loop:" << endl;
        for (auto iter : read_mb) {
		RfidParseUR parseUR(iter, RFID_MB_TID);
		cout << parseUR << endl;
	}
	read_mb.clear();

	cout << "ret=" << ret << endl;;
}


void start_test(RfidInterface& rf, vector<string>& summary)
{
	int loop_count = 0;
	bool expired = false;
	auto start = std::chrono::system_clock::now();
	do {
		rf.InventoryEPC(1, false, summary);

		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		expired = (elapsed_seconds.count() > cmdOpt.looptime);
		loop_count++;
		cout << (int)(elapsed_seconds.count()) << "\r" << flush;
	} while (!expired);

	cout << endl << "loop_count: " << loop_count << endl;
}


void statistic(vector<string> &summary)
{
	ofstream outfile;
	outfile.open(cmdOpt.save_file, ios::out | ios::trunc );

	for (vector<string>::const_reference elem : summary) {
		RfidParseU parser(elem);
		if ( parser.has_data ) {
			outfile << parser.time.min << " "
				<< parser.time.sec << " "
				<< parser.time.ms << " "
				<< parser.antenna << " "
				<< parser.epc.epc << endl;
		}
	}
	outfile.close();
}

int main(int argc, char** argv)
{
	vector<string> args(argv + 1, argv + argc);
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::info);
        PQParams pq_params = {
		.ip_type = IP_TYPE_IPV4, // IP_TYPE_IPV(4|6)
		.port = 1001 // default 1001
	};
	sprintf(pq_params.ip_addr, "192.168.88.91");
	print_endian();
	RfidInterface rf;
	vector<string> summary{};

        // Loop over command-line args
	// (Actually I usually use an ordinary integer loop variable and compare
	// args[i] instead of *i -- don't tell anyone! ;)
	for (auto i = args.begin(); i != args.end(); ++i) {
		string opt;
		if (*i == "-h" || *i == "--help") {
			cout << "Syntax: door -p <power> -t <looptime>" << endl;
			return 0;
		} else if (*i == "-p") {
			opt = *++i;
			cmdOpt.power = std::stoi(opt);
		} else if (*i == "-t") {
			opt = *++i;
			cmdOpt.looptime = std::stoi(opt);
		} else if (*i == "-f") {
			opt = *++i;
			cmdOpt.save_file = opt;
		}
	}
	cout << "cmdOpt.power = " << cmdOpt.power << endl;
	cout << "cmdOpt.looptime = " << cmdOpt.looptime << endl;

        start_test(rf, summary);
	statistic(summary);
}