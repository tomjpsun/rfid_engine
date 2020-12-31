#include <iostream>
#include <exception>
#include <chrono>
#include <thread>
#include <algorithm>

#include "async_socket.hpp"
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
	CmdHandler cmdHandler("192.168.88.91", 1001, std::atoi(loop_option.c_str()));
	vector<uint8_t> cmd;

	// get version
	cmd = vector<uint8_t>{0x0A, 'V', 0x0D};
	cmdHandler.send(cmd);
	this_thread::sleep_for(100ms);

	// get reader ID
	cmd = vector<uint8_t>{0x0A, 'S', 0x0D};
	cmdHandler.send(cmd);
	this_thread::sleep_for(100ms);

	// get tag EPC [ U,R2,0,6 ]
	cmd = vector<uint8_t>{0x0A, 'U', ',' , 'R', '2', ',', '0', ',', '6', 0x0D};
	//cmd = vector<uint8_t>{0x0A, '@', 'U', 0x0D};
	cmdHandler.send(cmd);
	this_thread::sleep_for(100ms);

        // get power
	//cmd = vector<uint8_t>{0x0A, 0x4E, 0x31, 0x2C, 0x30, 0x41, 0x0D};
	//cmdHandler.send(cmd);
	//this_thread::sleep_for(100ms);

	return 0;
}
