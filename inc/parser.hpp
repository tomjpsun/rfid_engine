#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <string>
#include <regex>
#include <iostream>
#include "parse_ds.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;


namespace rfid {
	class RfidParseEPC
	{
	public: RfidParseEPC() {}
		RfidParseEPC(const string response);
		friend std::ostream& operator<< (std::ostream& os, const RfidParseEPC& parseEPC);

		bool is_match;
		string proto;
		string epc;
		string crc;
	};

	class RfidTime
	{
	public:
		RfidTime();
		RfidTime(const string time);
		bool is_match;
		int year;
		int month;
		int day;
		int hour;
		int min;
		int sec;
		int ms;
		friend std::ostream& operator<<(std::ostream &os, const RfidTime& parseTime);
	};


	class RfidParseR
	{
	public:
		RfidParseR(const string response);
		friend std::ostream& operator<<(std::ostream &os, const RfidParseR& parseR);
		bool is_match;
		bool has_data;
		RfidTime time;
		string antenna;
		string data;
	};


	class RfidParseU
	{
	public:
		RfidParseU(const string response);

		friend std::ostream& operator<<(std::ostream &os, const RfidParseU& parseR);
		bool is_match;
		bool has_data;
		RfidTime time;
		string antenna;
		string data;
		RfidParseEPC epc;
	};



	class RfidParseUR
	{
	public:
		RfidParseUR(const string response, const int bank);

		friend std::ostream& operator<<(std::ostream &os, const RfidParseUR& parseUR);
		bool is_match;
		bool has_data;
		RfidTime time;
		string antenna;
		string data;
		RfidParseEPC epc;
		string tid;
		string user;
		string err; // 0, 3, 4, B, F
	};


	void to_json(json& j, const RfidTime& time);
	void from_json(const json& j, RfidTime& time);
	void to_json(json& j, const RfidParseEPC &pser);
	void from_json(const json& j, RfidParseEPC& pser);
	void to_json(json& j, const RfidParseUR& pser);
	void from_json(const json& j, RfidParseUR& pser);
	void to_json(json& j, const RfidParseR& pser);
	void from_json(const json& j, RfidParseR& pser);
	void to_json(json& j, const RfidParseU& pser);
	void from_json(const json& j, RfidParseU& pser);

	std::ostream &operator<<(std::ostream &os, const RfidParseEPC &parseEPC);
	std::ostream &operator<<(std::ostream &os, const RfidParseUR &parseUR);
	std::ostream &operator<<(std::ostream &os, const RfidParseR &parseR);
	std::ostream &operator<<(std::ostream &os, const RfidParseU &parseU);
	std::ostream &operator<<(std::ostream &os, const RfidTime &rfidTime);


} // namespace rfid

#endif // __PARSER_HPP__
