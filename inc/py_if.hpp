#ifndef __PY_IF_HPP__
#define __PY_IF_HPP__

#include <string>
#include <functional>

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
        // Python wrapper
	Foo* Foo_new();
        void Foo_bar(Foo* foo);
	void Foo_get_antenna_data(Foo* foo, Antenna_t cb_func);
	void Foo_get_coordinate(Foo* foo, Coordinate_t cb_func);
	void Foo_get_statistics(Foo* foo, Statistics_t cb_func);

}

#endif
