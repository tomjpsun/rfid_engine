#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "common.hpp"

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
	void bar() {
		std::cout << "Hello" << std::endl;
	}

	void get_antenna_data(Antenna_t antenna_cb) {
		string filename = "/home/tom/work/rfid_manager/sample_data/Antenna.csv";
		cout << "print_file(" << filename << "):" << endl;
		ifstream file(filename);
		string buf;
		for ( ; getline(file, buf); ) {
			antenna_cb(buf.data(), buf.size());
		}
		cout << endl;
	}
	void get_coordinate(Coordinate_t coordinate_cb) {
		string filename = "/home/tom/work/rfid_manager/sample_data/Coordinate.csv";

	}
	void get_statistics(Statistics_t statistics_cb) {
		string filename = "/home/tom/work/rfid_manager/sample_data/Statistics.csv";

	}
};

extern "C"
{
	Foo* Foo_new() { return new Foo(); }
	void Foo_bar(Foo* foo) { foo->bar(); }
	void Foo_get_antenna_data(Foo* foo, Antenna_t cb_func) { foo->get_antenna_data(cb_func); }
	void Foo_get_coordinate(Foo* foo, Coordinate_t cb_func) { foo->get_coordinate(cb_func); }
	void Foo_get_statistics(Foo* foo, Statistics_t cb_func) { foo->get_statistics(cb_func); }
}
