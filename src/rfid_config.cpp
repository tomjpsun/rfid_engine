#include <iostream>
#include "rfid_config.hpp"

namespace rfid
{
	void to_json(json& j, const RfidConfig& cfg) {
		j = json { { "log_file", cfg.log_file },
			   { "log_level", cfg.log_level },
			   { "reader_info_list", cfg.reader_info_list }
		};
	}

        void from_json(const json& j, RfidConfig& cfg) {
		j.at( "log_file" ).get_to( cfg.log_file );
                j.at( "log_level" ).get_to( cfg.log_level );
		j.at( "reader_info_list" ).get_to( cfg.reader_info_list );
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
