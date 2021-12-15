#include "ulog_type.h"
#include "nlohmann/json.hpp"


using json = nlohmann::json;

namespace ulog_namespace {

	void to_json(json& j, const Ulog& log) {
		j = json{ {"logger_id", log.logger_id},
			  {"log_level", log.log_level},
			  {"msg", log.msg} };
	}
	void from_json(const json& j, Ulog& log) {
		j.at("logger_id").get_to(log.logger_id);
		j.at("log_level").get_to(log.log_level);
		j.at("msg").get_to(log.msg);
	}
}
