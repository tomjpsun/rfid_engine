#include <iostream>
#include "rfid_config.hpp"

namespace rfid
{
	void to_json(json& j, const RfidConfig& cfg) {
		j = json { { "log_file", cfg.log_file },
			   { "log_level", cfg.log_level } };
	}

        void from_json(const json& j, RfidConfig& cfg) {
		j.at( "log_file" ).get_to( cfg.log_file );
		j.at( "log_level" ).get_to( cfg.log_level );
	}

        void to_json(json& j, const ReaderInfo info) {
		j = json { { "reader_ip", info.reader_ip },
			   { "reader_port", info.reader_port },
			   { "reader_id" , info.reader_id } };
	}

	void from_json(const json& j, ReaderInfo& info) {
		j.at( "reader_ip" ).get_to( info.reader_ip );
		j.at( "reader_port" ).get_to( info.reader_port );
		j.at( "reader_id" ).get_to( info.reader_id );
		std::cout << "ReaderInfo from_json(), reader_ip = " << info.reader_ip << endl;
	}

}
