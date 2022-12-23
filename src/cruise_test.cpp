#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <sstream>
#include <iostream>
#include <iomanip>
#include <utility>
#include <chrono>
#include <time.h>
#include <stdio.h>
#include <time.h>

#include "aixlog.hpp"
#include "cruise_type.hpp"
#include "catch_amalgamated.hpp"

using namespace std;
using namespace cruise_namespace;
using namespace std::chrono;

template<typename Clock, typename Duration>
std::ostream &operator<<(std::ostream &stream,
			 const std::chrono::time_point<Clock, Duration> &time_point) {
	const time_t time = Clock::to_time_t(time_point);
#if __GNUC__ > 4 ||							\
	((__GNUC__ == 4) && __GNUC_MINOR__ > 8 && __GNUC_REVISION__ > 1)
	// Maybe the put_time will be implemented later?
	struct tm tm;
	localtime_r(&time, &tm);
	return stream << std::put_time(&tm, "%c"); // Print standard date&time
#else
	char buffer[26];
	ctime_r(&time, buffer);
	buffer[24] = '\0';  // Removes the newline that is added
	return stream << buffer;
#endif
}

// for debug purpose
inline void print_timepoint(system_clock::time_point timepoint) {
	std::cout << timepoint << std::endl;
}


SCENARIO( "Test Cruise::time_point()" ) {
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug);
	GIVEN( "A default time point" ) {
		Cruise cruise{}; // default 2000-01-01 00:00:00.123456"
		cruise.timestamp = cruise.build_timestamp();
		WHEN( "print timepoint" ) {
			THEN ("no idea") {
				cout << "time stamp = " << cruise.timestamp << endl;
				print_timepoint(cruise.time_point());
				REQUIRE( true );
			}
		}
	}
}
SCENARIO( "Test Cruise::convert_to_local_time()" ) {
	GIVEN( "A default time point" ) {
		Cruise cruise{}; // default 2000-01-01 00:00:00.123456"
		cruise.convert_to_local_time();
		WHEN( "print timepoint" ) {
			THEN ("no idea") {
				cout << "time stamp = " << cruise.timestamp << endl;
				print_timepoint(cruise.time_point());
				REQUIRE( true );
			}
		}
	}
}
