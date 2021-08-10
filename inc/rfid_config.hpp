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
		string type;
		vector<string> settings;
		string reader_id;
	};

	class RfidConfig
	{
	public:
		string log_file;
		int log_level;
		std::vector<ReaderInfo> reader_info_list;
		bool enable_watch_dog;
	};

	class RfidConfigFactory
	{
	public:
		RfidConfigFactory();
		static RfidConfig get_config();
	private:
		static RfidConfig cfg;
		string default_path_name;
	};

	void to_json(json& j, const ReaderInfo);
	void from_json(const json& j, ReaderInfo& readerInfo);
        void to_json(json& j, const RfidConfig& cfg);
	void from_json(const json& j, RfidConfig& cfg);
}

#endif // __RFID_CONFIG_HPP__
