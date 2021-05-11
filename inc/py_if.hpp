#ifndef __PY_IF_HPP__
#define __PY_IF_HPP__

#include <string>
#include <functional>
#include <memory>
#include "rfid_if.hpp"

using namespace std;

extern "C"
{
	typedef void(*Antenna_t)(const char* antenna_csv_data, int size);
	typedef void(*Coordinate_t)(const char* coordinate_csv_data, int size);
	typedef void(*Statistics_t)(const char* statistics_csv_data, int size);
}

class Foo
{
public:
	Foo(char* ip, int port) {
		PQParams pqParams;
		sprintf(pqParams.ip_addr, (const char*)ip);
		pqParams.port = port;
		pqParams.ip_type = IP_TYPE_IPV4;
		p_rf = shared_ptr<RfidInterface>(new RfidInterface{ pqParams });
	}
	void bar();
	void simulate_callback(string filename,
			       std::function<void(const char*, int)> cb_func);
	// read antenna data and pass them to antenna_cb()
	void get_antenna_data(Antenna_t antenna_cb);

        // read coordinate data and pass them to coordinate_cb()
	void get_coordinate(Coordinate_t coordinate_cb);

	// read statistics data and pass them to statistics_cb()
	void get_statistics(Statistics_t statistics_cb);

	int InventoryEPC(int nSlotQ, bool fLoop, char* InventoryEPCjson) {
		vector<string> result;
		int ret = p_rf->InventoryEPC(nSlotQ, fLoop, result);

		return ret;
	}

	shared_ptr<RfidInterface> p_rf;
};


extern "C"
{
        // Python wrapper
	Foo* Foo_new(char* ip_addr, int port) { return new Foo(ip_addr, port); };
        void Foo_bar(Foo* foo);
	void Foo_get_antenna_data(Foo* foo, Antenna_t cb_func);
	void Foo_get_coordinate(Foo* foo, Coordinate_t cb_func);
	void Foo_get_statistics(Foo* foo, Statistics_t cb_func);

}

#endif
