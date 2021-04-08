#include "cpp_if.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <climits>
#include <condition_variable>
#include "common.hpp"
#include "cmd_handler.hpp"
#include "packet_content.hpp"
#include "observer.hpp"
#include "aixlog.hpp"
#include "send_sync_observer.hpp"
#include "py_if.hpp"

using namespace std;
using namespace rfid;


void Foo::bar() {
	std::cout << "Hello" << std::endl;
}

void Foo::simulate_callback(string filename,
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
void Foo::get_antenna_data(Antenna_t antenna_cb) {
	string filename = "/home/tom/work/rfid_manager/sample_data/Antenna.csv";
	std::function<void(const char*, int)> cb_func(antenna_cb);
	simulate_callback(filename, cb_func);
}

	// read coordinate data and pass them to coordinate_cb()
void Foo::get_coordinate(Coordinate_t coordinate_cb) {
	string filename = "/home/tom/work/rfid_manager/sample_data/Coordinate.csv";
	std::function<void(const char*, int)> cb_func(coordinate_cb);
	simulate_callback(filename, cb_func);
}

	// read statistics data and pass them to statistics_cb()
void Foo::get_statistics(Statistics_t statistics_cb) {
	string filename = "/home/tom/work/rfid_manager/sample_data/Statistics.csv";
	std::function<void(const char*, int)> cb_func(statistics_cb);
	simulate_callback(filename, cb_func);
}
