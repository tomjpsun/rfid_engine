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



class RfidParseR
{
public:
	RfidParseR(const string response);

	friend ostream& operator<<(ostream &os, const RfidParseR& parseR);
	bool is_match;
	bool has_data;
	string time;
	string antenna;
	string data;
};



class RfidParseUR
{
public:
	RfidParseUR(const string response, const int bank);

	friend ostream& operator<<(ostream &os, const RfidParseR& parseR);
	bool is_match;
	bool has_data;
	string time;
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


#endif // __PARSER_HPP__
