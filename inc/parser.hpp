#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <string>
#include <regex>
#include <iostream>
#include "parse_ds.hpp"

using namespace std;



class RfidParseEPC
{
public: RfidParseEPC() {}
	RfidParseEPC(const string response);
	friend ostream& operator<< (ostream& os, const RfidParseEPC& parseEPC);

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
	friend ostream& operator<<(ostream &os, const RfidTime& parseTime);
};


class RfidParseR
{
public:
	RfidParseR(const string response);
	friend ostream& operator<<(ostream &os, const RfidParseR& parseR);
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

	friend ostream& operator<<(ostream &os, const RfidParseU& parseR);
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

	friend ostream& operator<<(ostream &os, const RfidParseR& parseR);
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




ostream &operator<<(ostream &os, const RfidParseEPC &parseEPC);
ostream &operator<<(ostream &os, const RfidParseUR &parseUR);
ostream &operator<<(ostream &os, const RfidParseR &parseR);
ostream &operator<<(ostream &os, const RfidParseU &parseU);
ostream &operator<<(ostream &os, const RfidTime &rfidTime);

#endif // __PARSER_HPP__
