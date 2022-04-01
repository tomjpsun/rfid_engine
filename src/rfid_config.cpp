#include <iostream>
#include <fstream>
#include "rfid_config.hpp"
#include "nlohmann/json.hpp"

rfid::RfidConfig rfid::RfidConfigFactory::cfg;

namespace rfid
{
	RfidConfigFactory::RfidConfigFactory()
	{

#ifdef __linux__
		config_path_name = "/etc/rfidengine/rfid_config.json";
#elif _WIN32
#else
		config_path_name = "/etc/rfidengine/rfid_config.json";
#endif

		std::ifstream i(config_path_name);
		if ( i.good() ) {
			json j;
			i >> j;
			cfg = j;
		} else {
			cout << "Cannot Find Config File: " << config_path_name << endl;
			exit(-1);
		}

#ifdef __linux__
		ifstream ifs( "/etc/machine-id" );
		std::string content( (std::istreambuf_iterator<char>(ifs) ),
				     (std::istreambuf_iterator<char>()) );
		content.pop_back(); // remove trailing CR char
		machine_id = content;
#elif _WIN32
		machine_id = "win-system-id"
#else
		machine_id = "non-linux-win-system-id";
#endif

	}

	RfidConfig RfidConfigFactory::get_config()
	{
		return cfg;
	}

	string RfidConfigFactory::get_machine_id()
	{
		return machine_id;
	}

	void to_json(json& j, const RfidConfig& cfg) {
		j = json { { "version_major", cfg.version_major },
			   { "version_minor", cfg.version_minor },
			   { "version_subminor", cfg.version_subminor },
			   { "dbg_en", cfg.dbg_en },
			   { "log_file", cfg.log_file },
			   { "log_level", cfg.log_level },
			   { "enable_watch_dog", cfg.enable_watch_dog },
			   { "en_log_cout", cfg.en_log_cout },
			   { "en_log_file", cfg.en_log_file },
			   { "en_log_syslog", cfg.en_log_syslog },
			   { "en_log_ulog", cfg.en_log_ulog },
			   { "ulog_server_ip", cfg.ulog_server_ip },
			   { "ulog_server_port", cfg.ulog_server_port }
		};
	}

        void from_json(const json& j, RfidConfig& cfg) {
		j.at( "version_major" ).get_to( cfg.version_major );
		j.at( "version_minor" ).get_to( cfg.version_minor );
		j.at( "version_subminor" ).get_to( cfg.version_subminor );
		j.at( "dbg_en" ).get_to( cfg.dbg_en );
		j.at( "log_file" ).get_to( cfg.log_file );
                j.at( "log_level" ).get_to( cfg.log_level );
		j.at( "enable_watch_dog" ).get_to( cfg.enable_watch_dog );
		j.at( "en_log_cout" ).get_to( cfg.en_log_cout );
		j.at( "en_log_file" ).get_to( cfg.en_log_file );
		j.at( "en_log_syslog" ).get_to( cfg.en_log_syslog );
		j.at( "en_log_ulog" ).get_to( cfg.en_log_ulog );
		j.at( "ulog_server_ip" ).get_to( cfg.ulog_server_ip );
		j.at( "ulog_server_port" ).get_to( cfg.ulog_server_port );
	}

}
