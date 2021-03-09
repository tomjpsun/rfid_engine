#ifndef _CPP_IF_HPP_
#define _CPP_IF_HPP_

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
	void bar();
	void simulate_callback(string filename,
			       std::function<void(const char*, int)> cb_func);
	// read antenna data and pass them to antenna_cb()
	void get_antenna_data(Antenna_t antenna_cb);

        // read coordinate data and pass them to coordinate_cb()
	void get_coordinate(Coordinate_t coordinate_cb);

	// read statistics data and pass them to statistics_cb()
	void get_statistics(Statistics_t statistics_cb);
};


extern "C"
{
	// CPP interface, PQ means Packet Queue

	void PQInit(PQParams pq_params);
	ssize_t PQSize();
        PacketContent PQPop();
	PacketContent PQPeek(int index);
	void PQReset();
	void PQSend(std::vector<uint8_t> cmd);
	void PQSendBuf(const void* buf, int length);
	void PQStartService();
	void PQStopService();

        // Python wrapper
	Foo* Foo_new();
        void Foo_bar(Foo* foo);
	void Foo_get_antenna_data(Foo* foo, Antenna_t cb_func);
	void Foo_get_coordinate(Foo* foo, Coordinate_t cb_func);
	void Foo_get_statistics(Foo* foo, Statistics_t cb_func);

}
#endif // _CPP_IF_HPP_
