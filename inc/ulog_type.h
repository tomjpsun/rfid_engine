#ifndef __ulog_type_h__
#define __ulog_type_h__

#include <string>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace ulog_namespace {
	class ulog {
	public:
		string logger_id;
		int log_level;
		string msg;
	};
	void to_json(json& j, const ulog& log);
	void from_json(const json& j, ulog& log);
}
#endif
