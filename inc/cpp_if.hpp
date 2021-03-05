#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <climits>
#include "common.hpp"
#include "cmd_handler.hpp"
#include "packet_content.hpp"
#include "observer.hpp"
#include "aixlog.hpp"

using namespace std;
using namespace rfid;

extern "C"
{
	typedef void(*Antenna_t)(const char* antenna_csv_data, int size);
	typedef void(*Coordinate_t)(const char* coordinate_csv_data, int size);
	typedef void(*Statistics_t)(const char* statistics_csv_data, int size);

#define IP_ADDR_LEN 16
#define IP_TYPE_IPV4 1
#define IP_TYPE_IPV6 2

        typedef struct _PQParams_ {
		char ip_addr[IP_ADDR_LEN];
		char ip_type; // IP_TYPE_IPV(4|6)
		int port; // default 1001
		int loop; // default 100
	}PQParams, *PPQParams;
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

static PQParams g_PQParams;
static CmdHandler g_CmdHandler;

extern "C"
{
	// CPP interface, PQ means Packet Queue

	void PQInit(PQParams pq_params) {
		AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::info);
		std::memcpy(&g_PQParams, &pq_params, sizeof(PQParams));
		PPQParams p = &g_PQParams;
                g_CmdHandler.start_recv_thread(p->ip_addr, p->port, p->loop);
	}

	ssize_t PQSize() {
		return g_CmdHandler.get_packet_queue()->size();
	}

        PacketContent PQPop() {
		PacketContent pkt;
		if (PQSize() > 0) {
			pkt = g_CmdHandler.get_packet_queue()->remove();
		}
		else {
			LOG(SEVERITY::ERROR) << __func__ << ", Packet Queue Size 0" << endl;
		}
		return pkt;
	}

	PacketContent PQPeek(int index = 0) {
		PacketContent pkt;
		if (PQSize() > 0)
			pkt =  g_CmdHandler.get_packet_queue()->peek(index);
		else {
			LOG(SEVERITY::DEBUG) << __func__ << ", Packet Queue Size 0" << endl;
		}
		return pkt;
	}


	void PQReset() {
		return g_CmdHandler.get_packet_queue()->reset();
	}

	void PQSend(std::vector<uint8_t> cmd) {
		g_CmdHandler.send(cmd);
	}

	void PQSendBuf(const void* buf, int length) {
		uint8_t* p = (uint8_t*) buf;
		std::vector<uint8_t> cmd(p, p + length);
		PQSend(cmd);
	}

	void PQStartService() {
		PPQParams p = &g_PQParams;
		g_CmdHandler.start_recv_thread( p->ip_addr, p->port, p->loop );
	}

	void PQStopService() {
		g_CmdHandler.stop_recv_thread();
	}

	// Python wrapper
	Foo* Foo_new() { return new Foo(); }
	void Foo_bar(Foo* foo) { foo->bar(); }
	void Foo_get_antenna_data(Foo* foo, Antenna_t cb_func) { foo->get_antenna_data(cb_func); }
	void Foo_get_coordinate(Foo* foo, Coordinate_t cb_func) { foo->get_coordinate(cb_func); }
	void Foo_get_statistics(Foo* foo, Statistics_t cb_func) { foo->get_statistics(cb_func); }


}
