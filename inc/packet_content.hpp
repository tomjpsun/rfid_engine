#ifndef _PACKET_CONTENT_HPP_
#define _PACKET_CONTENT_HPP_

#include <vector>
#include <string>
#include <iostream>

using namespace std;

class PacketContent {
public:
	// constructor converter from raw data
	PacketContent(const string& packet_data):
		vtBuffer(packet_data.begin(), packet_data.end()) {}

	// assignment converter from raw data
	PacketContent& operator=(const string& packet_data) {
		vtBuffer = vector<unsigned char>
			{ packet_data.begin(), packet_data.end() };
		return *this;
	}
	// converter to raw data
	operator std::string() {
		return std::string{vtBuffer.begin(), vtBuffer.end() };
	}

	PacketContent() {}
 	vector<unsigned char> vtBuffer;
};

ostream& operator<<(ostream& os, const PacketContent& pkt) {
	string s{pkt.vtBuffer.begin(), pkt.vtBuffer.end()};
	os << s;
	return os;
}

#endif
