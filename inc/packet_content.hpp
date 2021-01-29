#ifndef _PACKET_CONTENT_HPP_
#define _PACKET_CONTENT_HPP_

#include <vector>
#include <string>

using namespace std;

class PacketContent {
public:
	PacketContent(const string& packet_data):
		vtBuffer(packet_data.begin(), packet_data.end()) {}

	PacketContent& operator=(const string& packet_data) {
		vtBuffer = vector<unsigned char>
			{ packet_data.begin(), packet_data.end() };
		return *this;
	}
	PacketContent() {}
 	vector<unsigned char> vtBuffer;
};

#endif
