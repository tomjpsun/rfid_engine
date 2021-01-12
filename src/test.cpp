#include <iostream>
#include <exception>
#include <chrono>
#include <thread>
#include <algorithm>

#include "aixlog.hpp"
#include "common.hpp"
#include "cmd_handler.hpp"

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
	string loop_option{"1000"};

	if (argc == 1) {
		print_usage_hint();
		exit(0);
	}

	if (cmdOptionExists(argv, argv+argc, "-l"))
		loop_option = getCmdOption(argv, argv+argc, "-l");

	cout << "loop_option = " << loop_option << endl;


	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);

	CmdHandler cmdHandler;
	cmdHandler.start_recv_thread("192.168.88.91", 1001, std::atoi(loop_option.c_str()));
	vector<uint8_t> cmd;

	// get version
	cmd = vector<uint8_t>{0x0A, 'V', 0x0D};
	cmdHandler.send(cmd);
	this_thread::sleep_for(10ms);

	// get reader ID
	cmd = vector<uint8_t>{0x0A, 'S', 0x0D};
	cmdHandler.send(cmd);
	this_thread::sleep_for(10ms);

	// get tag EPC [ U,R2,0,6 ]
        // return:
	// Ex. @2021/01/04 10:38:53.842-Antenna1-U3000E28011606000020D6842CCCF6B9E,RE2801160200074CF085909AD
	// format : [time stamp]-[antenna no.]-U[protocol][EPC][CRC16],R[TID]
	// EPC: E28011606000020D6842CCCF
	// CRC16: 6B9E
	// TID: E2801160200074CF085909AD

	cmd = vector<uint8_t>{0x0A, '@', 'U', ',' , 'R', '2', ',', '0', ',', '6', 0x0D};
	//cmd = vector<uint8_t>{0x0A, '@', 'U', 0x0D};
	cmdHandler.send(cmd);

	// let CmdHandler wait for more response
	this_thread::sleep_for(5s);
	cmdHandler.stop_recv_thread();
	return 0;
}
