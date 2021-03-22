#include <iostream>
#include <exception>
#include <chrono>
#include <thread>
#include <algorithm>

#include "aixlog.hpp"
#include "common.hpp"
#include "cmd_handler.hpp"
#include "cpp_if.hpp"
#include "conn_queue.hpp"

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

	ConnQueue conn_queue(pq_params);
	conn_queue.start_service();

	vector<uint8_t> cmd;

	// get version
	cmd = vector<uint8_t>{0x0A, 'V', 0x0D};
	conn_queue.send(cmd);
	//this_thread::sleep_for(100ms);
	cout << conn_queue.pop().to_string();

	// get reader ID
	cmd = vector<uint8_t>{0x0A, 'S', 0x0D};
	conn_queue.send(cmd);
	//this_thread::sleep_for(100ms);
	cout << conn_queue.pop().to_string();

	// get tag EPC [ U,R2,0,6 ]
        // return:
	// Ex. @2021/01/04 10:38:53.842-Antenna1-U3000E28011606000020D6842CCCF6B9E,RE2801160200074CF085909AD
	// format : [time stamp]-[antenna no.]-U[protocol][EPC][CRC16],R[TID]
	// EPC: E28011606000020D6842CCCF
	// CRC16: 6B9E
	// TID: E2801160200074CF085909AD

	cmd = vector<uint8_t>{0x0A, '@', 'U', ',' , 'R', '2', ',', '0', ',', '6', 0x0D};
	//cmd = vector<uint8_t>{0x0A, '@', 'U', 0x0D};
	conn_queue.send(cmd);

	// let CmdHandler wait for more response
	//this_thread::sleep_for(3s);
	while (conn_queue.size() > 0)
		cout << conn_queue.pop().to_string();

	conn_queue.stop_service();
	return 0;
}
