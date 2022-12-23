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
#include "common.hpp"

using namespace std;
using namespace cruise_namespace;
using namespace std::chrono;


//SCENARIO( "Test Cruise::time_point()" ) {
//	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug);
//	GIVEN( "A default time point" ) {
//		Cruise cruise{}; // default 2000-01-01 00:00:00.123456"
//		cruise.timestamp = cruise.build_timestamp();
//		WHEN( "print timepoint" ) {
//			THEN ("no idea") {
//				cout << "time stamp = " << cruise.timestamp << endl;
//				print_timepoint(cruise.time_point());
//				REQUIRE( true );
//			}
//		}
//	}
//}
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
