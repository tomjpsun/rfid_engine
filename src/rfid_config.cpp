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
			   { "reader_info_list", cfg.reader_info_list },
			   { "enable_watch_dog", cfg.enable_watch_dog }
		};
	}

        void from_json(const json& j, RfidConfig& cfg) {
		j.at( "version_major" ).get_to( cfg.version_major );
		j.at( "version_minor" ).get_to( cfg.version_minor );
		j.at( "version_subminor" ).get_to( cfg.version_subminor );
		j.at( "dbg_en" ).get_to( cfg.dbg_en );
		j.at( "log_file" ).get_to( cfg.log_file );
                j.at( "log_level" ).get_to( cfg.log_level );
		j.at( "reader_info_list" ).get_to( cfg.reader_info_list );
		j.at( "enable_watch_dog" ).get_to( cfg.enable_watch_dog );
	}

        void to_json(json& j, const ReaderInfo info) {
		j = json { { "type", info.type },
			   { "settings", info.settings },
			   { "reader_id" , info.reader_id } };
	}

	void from_json(const json& j, ReaderInfo& info) {
		j.at( "type" ).get_to( info.type );
		j.at( "settings" ).get_to( info.settings );
		j.at( "reader_id" ).get_to( info.reader_id );
		if (info.type == "socket")
			std::cout << "ReaderInfo from_json(), reader_ip = " << info.settings[0]
				  << ", Port = " << info.settings[1] << endl;
		else
			std::cout << "ReaderInfo from_json, serial name = " << info.settings[0]
				  << endl;
	}
}
