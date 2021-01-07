#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "common.hpp"

class Foo
{
	enum { BatchSize = 10 };
public:
	void bar() {
		std::cout << "Hello" << std::endl;
	}
	void get_antenna_data() {

	}
	void get_coordinate() {

	}
	void get_statistics() {

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
