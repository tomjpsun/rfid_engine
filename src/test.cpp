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
		.port = 1001, // default 1001
		.loop = 100 // default 100
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


	result = rf.InventoryEPC(1, true, 5);

	return 0;
}
