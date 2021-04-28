#ifndef _PACKET_CONTENT_HPP_
#define _PACKET_CONTENT_HPP_

#include <vector>
#include <string>
#include <iostream>
#include "common.hpp"

using namespace std;
enum {
	PacketTypeNormal,
	PacketTypeHeartBeat
};

class PacketContent {
public:
	// constructor converter from raw data
	PacketContent(const string& packet_data, int pkt_type):
		vtBuffer(packet_data.begin(), packet_data.end()),
		packet_type(pkt_type){}

	// assignment converter from raw data
	PacketContent& operator=(const string& packet_data) {
		vtBuffer = vector<unsigned char>
			{ packet_data.begin(), packet_data.end() };
		return *this;
	}

	// converter to raw data
	std::string to_string() {
		std::string s{ vtBuffer.begin(), vtBuffer.end() };
		trim(s);
		return s;
	}

	operator char*() {
		return (char*)(vtBuffer.data());
	}

	size_t size() {
		return vtBuffer.size();
	}

	PacketContent() {}

	int get_packet_type() { return packet_type; }
	void set_packet_type(int new_packet_type) { packet_type = new_packet_type; }
 	vector<unsigned char> vtBuffer;
	int packet_type;
};


#endif
