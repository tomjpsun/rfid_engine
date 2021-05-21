#ifndef __RFID_CONFIG_HPP__
#define __RFID_CONFIG_HPP__

#include <string>
#include <vector>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace rfid {

	class ReaderInfo
	{
	public:
		string reader_ip;
		int reader_port;
		string reader_id;
	};

	class RfidConfig
	{
	public:
		string log_file;
		int log_level;
		std::vector<ReaderInfo> reader_info_list;
	};

	void to_json(json& j, const ReaderInfo);
	void from_json(const json& j, ReaderInfo& readerInfo);
        void to_json(json& j, const RfidConfig& cfg);
	void from_json(const json& j, RfidConfig& cfg);
}

#endif // __RFID_CONFIG_HPP__
