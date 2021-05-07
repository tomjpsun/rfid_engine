#include <string>
#include <regex>
#include <iostream>
#include "parse_ds.hpp"
#include "parser.hpp"

RfidParseEPC::RfidParseEPC(const string response) {
	const regex rgx( "([0-9a-fA-F]{4})([0-9a-fA-F]*)([0-9a-fA-F]{4})" );
	smatch index_match;
	is_match = std::regex_match(response, index_match, rgx);
	if ( is_match ){
		proto = index_match[1].str();
		epc = index_match[2].str();
		crc = index_match[3].str();
	}
}


RfidParseUR::RfidParseUR(const string response, const int bank) {
	const regex rgxUR( "^(@?)(\\d{4}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3})-Antenna(\\d+)-U(.*)$" );
	smatch index_match;
	is_match = std::regex_match(response, index_match, rgxUR);
	if ( is_match ){
		time = RfidTime(index_match[2].str());
		antenna = index_match[3].str();
		data = index_match[4].str();
	}
	has_data = ( data.size() > 0 );
	if (has_data) {

		smatch data_match;
		string part2; // part 2 of response
		const regex data_rgx("([0-9a-fA-F]*),R?(.*)");
		switch (bank) {
			case RFID_MB_TID: {
				if ( std::regex_match(data, data_match, data_rgx) ) {
					epc = RfidParseEPC(data_match[1].str());
					part2 = data_match[2].str();
					if ( part2.size()==1 )
						err = part2;
					else
						tid = part2;
				}
				break;
			}
			case RFID_MB_USER: {
				if ( std::regex_match(data, data_match, data_rgx) ) {
					epc = RfidParseEPC( data_match[1].str());
					part2 = data_match[2].str();
					if ( part2.size()==1 )
						err = part2;
					else
						user = part2;
				}
				break;
			}
			default:
				break;
		}
	}
}


RfidParseR::RfidParseR(const string response) {
	const regex rgx( "^(@?)(\\d{4}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3})-Antenna(\\d+)-R?(.*)$" );
	smatch index_match;
	is_match = std::regex_match(response, index_match, rgx);
	if ( is_match ){
		time = RfidTime(index_match[2].str());
		antenna = index_match[3].str();
		data = index_match[4].str();
	}
	has_data = ( data.size() > 0 );
}


RfidParseU::RfidParseU(const string response) {
	const regex rgx( "^(@?)(\\d{4}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3})-Antenna(\\d+)-U(.*)$" );
	smatch index_match;
	is_match = std::regex_match(response, index_match, rgx);
	if ( is_match ){
		time = RfidTime(index_match[2].str());
		antenna = index_match[3].str();
		data = index_match[4].str();
	}
	has_data = ( data.size() > 0 );
	if (has_data) {
		epc = RfidParseEPC(data);
	}
}


RfidTime::RfidTime() {}

RfidTime::RfidTime(const string time) {
	const regex rgx( "(\\d{4})/(\\d{2})/(\\d{2}) (\\d{2}):(\\d{2}):(\\d{2})\\.(\\d{3})" );
	smatch index_match;
	is_match = std::regex_match(time, index_match, rgx);
	if ( is_match ){
		year  = std::stoi(index_match[1].str());
		month = std::stoi(index_match[2].str());
		day   = std::stoi(index_match[3].str());
		hour  = std::stoi(index_match[4].str());
		min   = std::stoi(index_match[5].str());
		sec   = std::stoi(index_match[6].str());
		ms    = std::stoi(index_match[7].str());
	}
}



ostream &operator<<(ostream &os, const RfidParseUR &parseUR)
{
	os << "is_match: " << parseUR.is_match
	   << ", has_data: " << parseUR.has_data
	   << ", time: " << parseUR.time
	   << ", antenna: " << parseUR.antenna
	   << ", data: " << parseUR.data
	   << ", epc: " << parseUR.epc
	   << ", tid: " << parseUR.tid
	   << ", user: " << parseUR.user
	   << ", err: " << parseUR.err;
	return os;
}

ostream &operator<<(ostream &os, const RfidParseR &parseR)
{
	os << "is_match: " << parseR.is_match
	   << ", has_data: " << parseR.has_data
	   << ", time: " << parseR.time
	   << ", antenna: " << parseR.antenna
	   << ", data: " << parseR.data;
	return os;
}

ostream &operator<<(ostream &os, const RfidTime &parseTime)
{
	os << "year: " << parseTime.year
	   << ", month: " << parseTime.month
	   << ", day: " << parseTime.day
	   << ", hour: " << parseTime.hour
	   << ", min: " << parseTime.min
	   << ", sec: " << parseTime.sec
	   << ", ms: " << parseTime.ms
	   << endl;

	return os;
}


ostream &operator<<(ostream &os, const RfidParseU &parseU)
{
	os << "is_match: " << parseU.is_match
	   << ", has_data: " << parseU.has_data
	   << ", time: " << parseU.time
	   << ", antenna: " << parseU.antenna
	   << ", data: " << parseU.data
	   << ", epc: " << parseU.epc;
	return os;
}

ostream &operator<<(ostream &os, const RfidParseEPC &parseEPC)
{
	os << " [EPC] is_match: " << parseEPC.is_match
	   << ", proto: " << parseEPC.proto
	   << ", epc: " << parseEPC.epc
	   << ", crc: " << parseEPC.crc;
	return os;
}
