#include "ulog_type.h"
#include "nlohmann/json.hpp"


using json = nlohmann::json;

namespace ulog_namespace {

	void to_json(json& j, const Ulog& log) {
		j = json{ {"logger_id", log.logger_id},
			  {"log_level", log.log_level},
			  {"msg", log.msg},
			  {"app_name", log.app_name},
			  {"app_flags", log.app_flags}};
	}
	void from_json(const json& j, Ulog& log) {
		j.at("logger_id").get_to(log.logger_id);
		j.at("log_level").get_to(log.log_level);
		j.at("msg").get_to(log.msg);
		j.at("app_name").get_to(log.app_name);
		j.at("app_flags").get_to(log.app_flags);
	}
}
