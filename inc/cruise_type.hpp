#ifndef __cruise_type_hpp__
#define __cruise_type_hpp__

#include <stdio.h>
#include <time.h>

#include <regex>
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include "nlohmann/json.hpp"
#include "c_if.h"


using namespace std;
using json = nlohmann::json;


namespace cruise_namespace {

/*
  Interface Requirements for Cruise :
  can access the 'count' 'epc' 'ms' member data
  support cast to 'time_t'
  support time_point()
*/

	class Cruise {
	public:

		Cruise(string epc="111122223333444455556666",
		       string tid="111122223333444455556666",
		       string readerID="07K5",
		       int count = 0,
		       int antenna = 0,
		       string timestamp = "2000-01-01 00:00:00.123456",
		       bool uploaded = false,
		       int appID = 0,
		       float rssi = -0.0):
			epc(epc), tid(tid), readerID(readerID),
			count(count), antenna(antenna), timestamp(timestamp),
			uploaded(uploaded), appID(appID), rssi(rssi)
			{
				parse_time_stamp();
			}

                Cruise(const Cruise& o) {
			epc = o.epc;
			tid = o.tid;
			readerID = o.readerID;
			count = o.count;
			antenna = o.antenna;
			timestamp = o.timestamp;
			uploaded = o.uploaded;
			appID = o.appID;
			rssi = o.rssi;
			id = o.id;
			created_at = o.created_at;
			year = o.year;
			month = o.month;
			day = o.day;
			hour = o.hour;
			min = o.min;
			sec = o.sec;
			ms = o.ms;
			timestamp = build_timestamp();
		}

		Cruise(const RFID_EPC_STATISTICS& stat) {
			epc = string(stat.epc);
			tid = string(stat.tid);
			readerID = string(stat.readerID);
			count = stat.count;
			antenna =stat.antenna;
			year = stat.year;
			month = stat.month;
			day = stat.day;
			hour = stat.hour;
			min = stat.min;
			sec = stat.sec;
			ms = stat.ms;
			timestamp = build_timestamp();
			uploaded = false;
			appID = 0;
			rssi = -0.0;
		}

		void convert_to_local_time() {
			// get GMT time point
                        std::time_t t_c = std::chrono::system_clock::to_time_t(
                                time_point() );
			// add local offset
			t_c = t_c + tz_offset();

			struct tm* plocal = std::localtime(&t_c);
                        year = plocal->tm_year + 1900;
                        month = plocal->tm_mon + 1;
                        day = plocal->tm_mday;
                        hour = plocal->tm_hour;
                        min = plocal->tm_min;
                        sec = plocal->tm_sec;
                        timestamp = build_timestamp();
		}

                int tz_offset() {
                        time_t gmt, rawtime = time(NULL);
                        struct tm *ptm;

#if !defined(WIN32)
                        struct tm gbuf;
                        ptm = gmtime_r(&rawtime, &gbuf);
#else
                        ptm = gmtime(&rawtime);
#endif
                        // Request that mktime() looksup dst in timezone database
                        ptm->tm_isdst = -1;
                        gmt = mktime(ptm);

                        return (int)difftime(rawtime, gmt);
		}

		string epc;
		string tid;
		string readerID;
		int count;
		int antenna;
		string timestamp;
		bool uploaded;
		// appID: scale for multiple applications, default = 0
		int appID;
		float rssi;

// for Database used, not converted to JSOn

                int id;
		string created_at;

// for Access control use, interface requires this member

                int year;
		int month;
		int day;
		int hour;
		int min;
		int sec;
                int ms;

                string build_timestamp() {
			const int TimeStampLength = 27;
			char buffer[TimeStampLength];
			std::memset(buffer, 0, TimeStampLength);
			snprintf(buffer, TimeStampLength,
				 "%04d-%02d-%02d %02d:%02d:%02d.%03d000",
				year, month, day,
				hour, min, sec, ms);
			return string(buffer);
		}

		void parse_time_stamp()	{
			const regex rgx( "(\\d{4})[\\/-](\\d{2})[\\/-](\\d{2}) (\\d{2}):(\\d{2}):(\\d{2}).(\\d{6})");
			smatch index_match;
			bool is_match = std::regex_match(timestamp, index_match, rgx);

			if ( is_match ) {
				string year_rx = index_match[1].str();
				string month_rx = index_match[2].str();
				string day_rx = index_match[3].str();
				string hour_rx = index_match[4].str();
				string min_rx = index_match[5].str();
				string sec_rx = index_match[6].str();
				string ms_rx = index_match[7].str();

				year = stoi(year_rx);
				month = stoi(month_rx); // Jan: 0
				day = stoi(day_rx);
				hour =  stoi(hour_rx);
				min =  stoi(min_rx);
				sec =  stoi(sec_rx);
				ms = stoi(ms_rx) / 1000; // Access control need this to calculate time
			}
		}


		std::chrono::system_clock::time_point make_time_point (int year, int mon, int day,
								     int hour, int min, int sec)
			{
				// convert given calander time to timepoint

				struct std::tm t;
				t.tm_sec = sec;        // second of minute (0 .. 59 and 60 for leap seconds)
				t.tm_min = min;        // minute of hour (0 .. 59)
				t.tm_hour = hour;      // hour of day (0 .. 23)
				t.tm_mday = day;       // day of month (1 .. 31)
				t.tm_mon = mon - 1;    // month of year (0 .. 11)
				t.tm_year = year - 1900; // year since 1900
				t.tm_isdst = -1;       // determine whether daylight saving time

                                // std::mktime output time count since epoch
				// but it treats input as LOCAL calendar time
				// and get epoch ( concept like UTC )
				std::time_t my_time_t = std::mktime(&t);
				if (my_time_t == -1) {
					throw "no valid system time";
				}
				// from_time_t() compensates LOCAL back to UTC
				return std::chrono::system_clock::from_time_t(my_time_t);
			}

		std::chrono::time_point<std::chrono::system_clock> time_point() {
			std::chrono::system_clock::time_point tp = make_time_point (
				year, month, day, hour, min, sec);
			tp += std::chrono::milliseconds{ ms };
			return tp;
		}

	};

	inline void to_json(json& j, const Cruise& cruise) {
		j = json{ {"epc", cruise.epc},
			  {"tid", cruise.tid},
			  {"readerID", cruise.readerID},
			  {"count", cruise.count},
			  {"antenna", cruise.antenna},
			  {"timestamp", cruise.timestamp},
			  {"uploaded", cruise.uploaded},
			  {"appID", cruise.appID},
			  {"rssi", cruise.rssi}
		};
	}
	inline void from_json(const json& j, Cruise& cruise) {
		j.at("epc").get_to(cruise.epc);
		j.at("tid").get_to(cruise.tid);
		j.at("readerID").get_to(cruise.readerID);
		j.at("count").get_to(cruise.count);
		j.at("antenna").get_to(cruise.antenna);
		j.at("timestamp").get_to(cruise.timestamp);
		j.at("uploaded").get_to(cruise.uploaded);
		j.at("appID").get_to(cruise.appID);
		j.at("rssi").get_to(cruise.rssi);
	}

}
#endif // __cruise_type_hpp__
