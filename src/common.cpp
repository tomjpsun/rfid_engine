#include <sstream>
#include <cstring>
#include "common.hpp"
#include <stdio.h>  /* defines FILENAME_MAX */
// #define WINDOWS  /* uncomment this line to use it for windows.*/
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include<iostream>
#include "packet_content.hpp"

using namespace std;


std::string get_current_dir()
{
	char buff[FILENAME_MAX];
	GetCurrentDir( buff, FILENAME_MAX );
	std::string current_working_dir(buff);
	return current_working_dir;
}


std::string hex_dump(void* from, int count)
{
	uint8_t *start = (uint8_t *)from;
	ostringstream ostr;

	uint8_t table[16] = {
	    '0', '1', '2', '3', '4', '5', '6', '7',
	    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};

	if (start != nullptr) {
		for (int i=0; i<count; i++) {
			uint8_t value = *(start + i);
			ostr << " ";
			ostr <<  table[(value >> 4)] << table[value & 0x0F];
			if ((i+1)%16 == 0)
				ostr << endl;
		}
	}
	return ostr.str();
}

template<typename T>
void concat( vector<T>& dest, vector<T>& src )
{
	dest.insert(
		dest.end(),
		std::make_move_iterator(src.begin()),
		std::make_move_iterator(src.end())
		);
}

ostream& operator<<(ostream& os, const PacketContent& pkt) {
	string s{pkt.vtBuffer.begin(), pkt.vtBuffer.end()};
	os << s;
	return os;
}


void print_endian()
{
	int x = 1;
	char *y = (char *)&x;
        cout << "Little endian test = " << (*y == 1) << endl;
}

string iptostr(const int* ip, int version)
{
	char buf[30];
	std::memset(buf, 0, 30);
	if (version ==  4)
		sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	if (version == 6)
		sprintf(buf, "%d:%d:%d:%d:%d:%d", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5]);
	return string(buf);
}
