#include <iostream>
#include <exception>
#include <chrono>
#include <thread>
#include "async_socket.hpp"
#include "aixlog.hpp"
#include "common.hpp"
#include "cmd_handler.hpp"

using namespace std;
using namespace rfid;

int main(int argc, char** argv)
{
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
