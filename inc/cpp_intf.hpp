#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <climits>
#include "common.hpp"
#include "cmd_handler.hpp"
#include "packet_content.hpp"

using namespace std;
using namespace rfid;

extern "C"
{
	typedef void(*Antenna_t)(const char* antenna_csv_data, int size);
	typedef void(*Coordinate_t)(const char* coordinate_csv_data, int size);
	typedef void(*Statistics_t)(const char* statistics_csv_data, int size);
}

class Foo
{
public:
	void bar() {
		std::cout << "Hello" << std::endl;
	}

	void simulate_callback(string filename,
			       std::function<void(const char*, int)> cb_func) {
		cout << "read file ( " << filename << " ): " << endl;
		ifstream file(filename);
		string buf;
		for ( ; getline(file, buf); ) {
			cb_func(buf.data(), buf.size());
		}
		cout << endl;
	}

	// read antenna data and pass them to antenna_cb()
	void get_antenna_data(Antenna_t antenna_cb) {
		string filename = "/home/tom/work/rfid_manager/sample_data/Antenna.csv";
		std::function<void(const char*, int)> cb_func(antenna_cb);
		simulate_callback(filename, cb_func);
	}

	// read coordinate data and pass them to coordinate_cb()
	void get_coordinate(Coordinate_t coordinate_cb) {
		string filename = "/home/tom/work/rfid_manager/sample_data/Coordinate.csv";
		std::function<void(const char*, int)> cb_func(coordinate_cb);
		simulate_callback(filename, cb_func);
	}

	// read statistics data and pass them to statistics_cb()
	void get_statistics(Statistics_t statistics_cb) {
		string filename = "/home/tom/work/rfid_manager/sample_data/Statistics.csv";
		std::function<void(const char*, int)> cb_func(statistics_cb);
		simulate_callback(filename, cb_func);
	}
};

extern "C"
{
	// CPP interface, PQ means Packet Queue
	static CmdHandler gCmdHandler;

	void PQInit(std::string ip, int port=1001, int loop=INT_MAX) {
		AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::info);
		gCmdHandler.start_recv_thread(ip, port, loop);
	}

	PacketContent PQPop() {
		return gCmdHandler.get_packet_queue()->remove();
	}

	PacketContent PQPeek() {
		return gCmdHandler.get_packet_queue()->peek();
	}

	ssize_t PQSize() {
		return gCmdHandler.get_packet_queue()->size();
	}

	void PQReset() {
		return gCmdHandler.get_packet_queue()->reset();
	}

	void PQSend(std::vector<uint8_t> cmd) {
		gCmdHandler.send(cmd);
	}

	void PQStopService() {
		gCmdHandler.stop_recv_thread();
	}

	// Python wrapper
	Foo* Foo_new() { return new Foo(); }
	void Foo_bar(Foo* foo) { foo->bar(); }
	void Foo_get_antenna_data(Foo* foo, Antenna_t cb_func) { foo->get_antenna_data(cb_func); }
	void Foo_get_coordinate(Foo* foo, Coordinate_t cb_func) { foo->get_coordinate(cb_func); }
	void Foo_get_statistics(Foo* foo, Statistics_t cb_func) { foo->get_statistics(cb_func); }


}
