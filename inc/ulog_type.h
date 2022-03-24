#ifndef __ulog_type_h__
#define __ulog_type_h__

#include <string>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace ulog_namespace {
	class Ulog {
	public:
		Ulog(string logger_id, int log_level, string msg,
		     string app_name = "rfid_engine", int app_flags = 0):
			logger_id(logger_id), log_level(log_level), msg(msg),
			app_name(app_name), app_flags(app_flags)
			{ }
		string logger_id;
		int log_level;
		string msg;
		string app_name;
		int app_flags;
	};
	void to_json(json& j, const Ulog& log);
	void from_json(const json& j, Ulog& log);
}
#endif
