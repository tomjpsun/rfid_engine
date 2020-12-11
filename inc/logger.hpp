#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_


#include <string>
#include <fstream>
#include <iostream>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sinks.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sinks/unlocked_frontend.hpp>


namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
using sink_t = sinks::synchronous_sink< sinks::text_ostream_backend >;
using p_backend_t = boost::shared_ptr< sinks::text_ostream_backend >;



class esl_logger
{
public:
	p_backend_t backend;
	boost::shared_ptr< std::ostream > file_ostream;
	boost::shared_ptr< std::ostream > sys_ostream;
	boost::shared_ptr< sink_t > sink;
	std::string ostream_file_name;
};

using p_logger_t = std::shared_ptr<esl_logger>;

p_logger_t init_logging(std::string fpath, int level = 0);
void esl_reset_log(p_logger_t p_logger);

#endif
