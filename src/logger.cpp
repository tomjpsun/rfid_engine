#include "logger.hpp"

BOOST_LOG_ATTRIBUTE_KEYWORD(log_severity, "Severity", boost::log::trivial::severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(log_timestamp, "TimeStamp", boost::posix_time::ptime)

p_logger_t init_logging(std::string fname, int level)
{
	p_logger_t plog = p_logger_t(new esl_logger());

	boost::shared_ptr< logging::core > core = logging::core::get();
	std::vector<decltype(logging::trivial::trace)> 	level_map {
		logging::trivial::trace,
		logging::trivial::debug,
		logging::trivial::info,
	        logging::trivial::warning,
		logging::trivial::error,
		logging::trivial::fatal
	};
	core->set_filter (
		logging::trivial::severity >= level_map[level]
		);
	// Create a backend and attach a couple of streams to it
	boost::shared_ptr< sinks::text_ostream_backend > backend =
		boost::make_shared< sinks::text_ostream_backend >();

	plog->backend = backend;
	plog->sys_ostream = boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter());
	plog->file_ostream = boost::shared_ptr< std::ostream >(new std::ofstream(fname,std::ios_base::app));
	plog->ostream_file_name = fname;

	backend->add_stream( plog->sys_ostream );
	backend->add_stream( plog->file_ostream );

	// Enable auto-flushing after each log record written
	backend->auto_flush(true);

	// Wrap it into the frontend and register in the core.
	// The backend requires synchronization in the frontend.

	boost::shared_ptr< sink_t > sink(new sink_t(backend));
	plog->sink = sink;

	logging::formatter formatter=
		expr::stream
		<<"["<<expr::format_date_time(log_timestamp,"%Y-%m-%d %H:%M:%S") <<"]"
		<<" <"<<log_severity<<"> "<<expr::message;

	sink->set_formatter(formatter);
	core->add_sink(sink);

	logging::add_common_attributes();
	return plog;
}


void esl_reset_log(p_logger_t p_logger)
{
	std::string fname = p_logger->ostream_file_name;
	p_logger->backend->remove_stream(p_logger->file_ostream);
	p_logger->file_ostream = boost::shared_ptr< std::ostream >
		(new std::ofstream(fname,std::ios_base::trunc));
	p_logger->backend->add_stream(p_logger->file_ostream);
}
