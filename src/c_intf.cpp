#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "common.hpp"

using namespace std;

class Foo
{
	enum { BatchSize = 10 };
public:
	void bar() {
		std::cout << "Hello" << std::endl;
	}
	void print_file(std::string filename) {
		cout << "print_file(" << filename << "):" << endl;
		ifstream file(filename);
		string buf;
		while (file >> buf) { cout << buf; }
		cout << endl;
	}
	void get_antenna_data() {
		string filename = "/home/tom/work/rfid_manager/sample_data/Antenna.csv";
		print_file(filename);
	}
	void get_coordinate() {
		string filename = "/home/tom/work/rfid_manager/sample_data/Coordinate.csv";
		print_file(filename);
	}
	void get_statistics() {
		string filename = "/home/tom/work/rfid_manager/sample_data/Statistics.csv";
		print_file(filename);
	}
};

extern "C"
{
	Foo* Foo_new() { return new Foo(); }
	void Foo_bar(Foo* foo) { foo->bar(); }
	void Foo_get_antenna_data(Foo* foo) { foo->get_antenna_data(); }
	void Foo_get_coordinate(Foo* foo) { foo->get_coordinate(); }
	void Foo_get_statistics(Foo* foo) { foo->get_statistics(); }
}
