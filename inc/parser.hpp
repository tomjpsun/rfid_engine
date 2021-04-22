#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <string>
#include <regex>
#include <iostream>

using namespace std;

class RfidParseEPC
{
public:
	RfidParseEPC(string response) {
		const regex rgx( "([0-9a-fA-F]{4})([0-9a-fA-F]*)([0-9a-fA-F]{4})" );
		smatch index_match;
		is_match = std::regex_match(response, index_match, rgx);
		if ( is_match ){
			proto = index_match[1].str();
			epc = index_match[2].str();
			crc = index_match[3].str();
		}
	}
	friend ostream& operator<< (ostream& os, const RfidParseEPC& parseEPC);

	bool is_match;
        string proto;
        string epc;
	string crc;
};

ostream &operator<<(ostream &os, const RfidParseEPC &parseEPC)
{
	os << "is_match: " << parseEPC.is_match
	   << ", proto: " << parseEPC.proto
	   << ", epc: " << parseEPC.epc
	   << ", crc: " << parseEPC.crc;
	return os;
}

class RfidParseR
{
public:
	RfidParseR(string response) {
		const regex rgx( "^(@?)(\\d{4}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3})-Antenna(\\d+)-R(.*)$" );
		smatch index_match;
		is_match = std::regex_match(response, index_match, rgx);
		if ( is_match ){
			time = index_match[2].str();
			antenna = index_match[3].str();
			data = index_match[4].str();
		}
		has_data = ( data.size() > 0 );
	}

	friend ostream& operator<<(ostream &os, const RfidParseR& parseR);
	bool is_match;
	bool has_data;
	string time;
	string antenna;
	string data;
};

ostream &operator<<(ostream &os, const RfidParseR &parseR)
{
	os << "is_match: " << parseR.is_match
	   << ", has_data: " << parseR.has_data
	   << ", time: " << parseR.time
	   << ", antenna: " << parseR.antenna
	   << ", data: " << parseR.data;
	return os;
}


#endif // __PARSER_HPP__
