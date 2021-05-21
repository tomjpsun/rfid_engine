#include "rfid_config.hpp"

namespace rfid
{

	void to_json(json& j, const RfidConfig& cfg) {
		j = json { { "reader_ip", cfg.reader_ip },
			   { "reader_port", cfg.reader_port },
			   { "log_file", cfg.log_file },
			   { "log_level", cfg.log_level } };
	}
	void from_json(const json& j, RfidConfig& cfg) {
		j.at( "reader_ip" ).get_to( cfg.reader_ip );
		j.at( "reader_port" ).get_to( cfg.reader_port );
		j.at( "log_file" ).get_to( cfg.log_file );
		j.at( "log_level" ).get_to( cfg.log_level );
	}
}
