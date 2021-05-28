#include <string>
#include <regex>
#include <iostream>
#include "parse_ds.hpp"
#include "parser.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace rfid {
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
		if ( is_match ) {
			time = RfidTime(index_match[2].str());
			antenna = index_match[3].str();
			data = index_match[4].str();
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
			has_data = ( data.size() > 0 );
			if (has_data) {
				epc = RfidParseEPC{data};
			}
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



	std::ostream &operator<<(std::ostream &os, const RfidParseUR &parseUR)
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

	std::ostream &operator<<(std::ostream &os, const RfidParseR &parseR)
	{
		os << "is_match: " << parseR.is_match
		   << ", has_data: " << parseR.has_data
		   << ", time: " << parseR.time
		   << ", antenna: " << parseR.antenna
		   << ", data: " << parseR.data;
		return os;
	}

	std::ostream &operator<<(std::ostream &os, const RfidTime &parseTime)
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


	std::ostream &operator<<(std::ostream &os, const rfid::RfidParseU &parseU)
	{
		os << "is_match: " << parseU.is_match
		   << ", has_data: " << parseU.has_data
		   << ", time: " << parseU.time
		   << ", antenna: " << parseU.antenna
		   << ", data: " << parseU.data
		   << ", epc: " << parseU.epc;
		return os;
	}

	std::ostream& operator<< (std::ostream& os, const RfidParseEPC& parseEPC)
	{
		os << " [EPC] is_match: " << parseEPC.is_match
		   << ", proto: " << parseEPC.proto
		   << ", epc: " << parseEPC.epc
		   << ", crc: " << parseEPC.crc;
		return os;
	}


	void to_json(json& j, const RfidTime& time)
	{
		j = json{ {"is_match", time.is_match},
			  {"year", time.year},
			  {"month", time.month},
			  {"day", time.day},
			  {"hour", time.hour},
			  {"min", time.min},
			  {"sec", time.sec},
			  {"ms", time.ms} };
	}

	void from_json(const json& j, RfidTime& time)
	{
		j.at("is_match").get_to(time.is_match);
		j.at("year").get_to(time.year);
		j.at("month").get_to(time.month);
		j.at("day").get_to(time.day);
		j.at("hour").get_to(time.hour);
		j.at("min").get_to(time.min);
		j.at("sec").get_to(time.sec);
		j.at("ms").get_to(time.ms);
	}

	void to_json(json& j, const RfidParseEPC &pser)
	{
                j = json { {"is_match", pser.is_match},
			   {"proto", pser.proto},
			   {"epc", pser.epc},
			   {"crc", pser.crc} };
	}

	void from_json(const json& j, RfidParseEPC& pser)
	{
                j.at("is_match").get_to(pser.is_match);
		j.at("proto").get_to(pser.proto);
		j.at("epc").get_to(pser.epc);
		j.at("crc").get_to(pser.crc);

	}

	void to_json(json& j, const RfidParseUR& pser)
	{
		j = json { {"is_match", pser.is_match},
			   {"has_data", pser.has_data},
			   {"antenna", pser.antenna},
			   {"data", pser.data},
			   {"tid", pser.tid},
			   {"user", pser.user},
			   {"err", pser.err},
			   {"time", pser.time},
			   {"epc", pser.epc} };
	}

	void from_json(const json& j, RfidParseUR& pser)
	{
		j.at("is_match").get_to(pser.is_match);
		j.at("has_data").get_to(pser.has_data);
		j.at("time").get_to(pser.time);
		j.at("antenna").get_to(pser.antenna);
		j.at("data").get_to(pser.data);
		j.at("epc").get_to(pser.epc);
		j.at("tid").get_to(pser.tid);
		j.at("user").get_to(pser.user);
		j.at("err").get_to(pser.err);
	}


	void to_json(json& j, const RfidParseR& pser)
	{
		j = json{ {"is_match", pser.is_match},
			  {"has_data", pser.has_data},
			  {"antenna", pser.antenna},
			  {"data", pser.data},
			  {"time", pser.time} };
	}

	void from_json(const json& j, RfidParseR& pser)
	{
		j.at("is_match").get_to(pser.is_match);
		j.at("has_data").get_to(pser.has_data);
		j.at("time").get_to(pser.time);
		j.at("antenna").get_to(pser.antenna);
		j.at("data").get_to(pser.data);
	}

	void to_json(json& j, const RfidParseU& pser)
	{
		j = json { {"is_match", pser.is_match},
			   {"has_data", pser.has_data},
			   {"antenna", pser.antenna},
			   {"data", pser.data},
			   {"time", pser.time},
			   {"epc", pser.epc} };
	}


	void from_json(const json& j, RfidParseU& pser)
	{
		j.at("is_match").get_to(pser.is_match);
		j.at("has_data").get_to(pser.has_data);
		j.at("time").get_to(pser.time);
		j.at("antenna").get_to(pser.antenna);
		j.at("data").get_to(pser.data);
		j.at("epc").get_to(pser.epc);
	}

} // namespace rfid
