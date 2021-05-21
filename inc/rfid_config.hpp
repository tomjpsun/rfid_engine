#ifndef __RFID_CONFIG_HPP__
#define __RFID_CONFIG_HPP__

#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace rfid {
	class RfidConfig
	{
	public:
		string reader_ip;
		int reader_port;
		string log_file;
		int log_level;
	};

        void to_json(json& j, const RfidConfig& cfg);
	void from_json(const json& j, RfidConfig& cfg);
}

#endif // __RFID_CONFIG_HPP__
