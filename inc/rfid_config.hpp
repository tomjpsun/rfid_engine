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
		int version_major;
		int version_minor;
		int version_subminor;
		bool dbg_en;
		string log_file;
		int log_level;
		std::vector<ReaderInfo> reader_info_list;
		bool enable_watch_dog;
		bool en_log_cout;
		bool en_log_file;
		bool en_log_syslog;
		bool en_log_ulog;
		string ulog_server_ip;
		int ulog_server_port;
	};

	class RfidConfigFactory
	{
	public:
		RfidConfigFactory();
		static RfidConfig get_config();
		string get_machine_id();
	private:
		static RfidConfig cfg;
		string config_path_name;
		string machine_id;
	};

	void to_json(json& j, const ReaderInfo);
	void from_json(const json& j, ReaderInfo& readerInfo);
        void to_json(json& j, const RfidConfig& cfg);
	void from_json(const json& j, RfidConfig& cfg);
}

#endif // __RFID_CONFIG_HPP__
