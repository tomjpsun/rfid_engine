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
	cout << "Usage : --loop=1000 --sec=[1...2^32-1]" << endl;
	cout << "\t loop: read loop count, set -1 for infinite loop, default = 1000" << endl;
	cout << "\t sec: operation time for this command, default = 5(sec)" << endl;
	cout << "\t finish program on either condition is met" << endl;
	cout << "\t w/o options, show this hint" << endl;
}

int main(int argc, char** argv)
{
	if (argc == 1) {
		print_usage_hint();
		exit(0);
	}

	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
	CmdHandler cmdHandler("192.168.88.91", 1001);
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
