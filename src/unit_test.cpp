#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <sstream>
#include <iomanip>
#include <utility>
#include <catch2/catch.hpp>
#include <time.h>

#include "aixlog.hpp"
#include "cmd_handler.hpp"
#include "packet_queue.hpp"
#include "packet_content.hpp"
#include "rfid_if.hpp"

using namespace std;
using namespace rfid;


SCENARIO( "Test CmdHandler" ) {
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug);
	GIVEN( "data of 4 packets" ) {
		CmdHandler cmd;
		string data = "\x0a@2021/01/13 14:39:47.739-Antenna4-VD407,000015E8,CA,2\x0d\x0a";
		data.append("\x0a@2021/01/13 14:39:47.749-Antenna4-S000015E8\x0d\x0a");
		data.append("\x0a@2021/01/13 14:39:47.739-Antenna4-VD407,000015E8,CA,2\x0d\x0a");
	        data.append("\x0a@2021/01/13 14:39:47.749-Antenna4-S000015E8\x0d\x0a");
		WHEN( "parsing packets" ) {
			cmd.process_buffer_thread_func(data);
			THEN ("Size should be 4") {
				REQUIRE( cmd.packet_queue_size() == 4 );
			}
		}
	}

	GIVEN( "heartbeat packets" ) {
		string data = "3heartbeat00-16-44\x0a@2021/01/13 14:39:47.739-Antenna4-VD407,000015E8,CA,2\x0d\x0a";
		data.append("3heartbeat00-16-443heartbeat00-16-443heartbeat00-16-44");

		WHEN( "parsing packets contains heartbeats" ) {
			CmdHandler cmd;
			cmd.process_buffer_thread_func(data);
			THEN ("Size should be 5") {
				REQUIRE( cmd.packet_queue_size() == 5 );
			}
		}
		WHEN( "varify packet type" ) {
			CmdHandler cmd;
			cmd.process_buffer_thread_func(data);
			THEN( "Should Contains 4 heartbeat packet" ) {
				int count = 0;
				for (int i=0; i<cmd.get_packet_queue()->size(); i++) {
					std::shared_ptr<PacketQueue<PacketContent>>
						ppq = cmd.get_packet_queue();
					PacketContent pkt = ppq->peek(i);
					if (pkt.packet_type == PacketTypeHeartBeat)
						count++;
				}

				REQUIRE( count == 4 );
			}
		}
	}
}

SCENARIO( "Test PacketQueue" ) {
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
	GIVEN( "packet data" ) {
		vector<string> v{"0", "1", "2", "3", "4", "5"};
		WHEN( "test push_back()" ) {
			PacketQueue<PacketContent> pq;
			for (auto p: v) {
				PacketContent pkt{p, PacketTypeNormal};
				pq.push_back(pkt);
			}
			REQUIRE( pq.size() == v.size() );
		}
		WHEN( "test remove()" ) {
			PacketQueue<PacketContent> pq;
			for (auto &p: v) {
				PacketContent pkt{p, PacketTypeNormal};
				pq.push_back(pkt);
			}

			PacketContent pkt = pq.remove(3);
			REQUIRE( pq.size() == v.size()-1 );
			REQUIRE( pkt.to_string() == "3" );
		}
		WHEN( "test peak()" ) {
			PacketQueue<PacketContent> pq;
			for (auto &p: v) {
				PacketContent pkt{p, PacketTypeNormal};
				pq.push_back(pkt);
			}

			PacketContent pkt = pq.peek(3);
			REQUIRE( pq.size() == v.size() );
			REQUIRE( pkt.to_string() == "3" );
		}

		WHEN( "test observer" ) {
			class ConcreteObserver : public Observer
			{
			public:
				ConcreteObserver( const int state ) :
					observer_state( state ),
					update_count(0) {}

				~ConcreteObserver() {}

				int get_state() {
					return observer_state;
				}

				void update( Subject *subject )	{
					update_count++;
					int packet_type = subject->get_state();
					std::cout << "Observer " << observer_state
						  << ", Packet type " << packet_type
						  << " added." << std::endl;
				}
				int get_update_count() { return update_count; }
			private:
				int observer_state;
				int update_count;
			};


			PacketQueue<PacketContent> pq;

			ConcreteObserver obs1(1);
			ConcreteObserver obs2(2);

			// register ourself to subject in PacketQueue
			pq.attach( &obs1 );
			pq.attach( &obs2 );

			for (auto &p: v) {
				PacketContent pkt{p, PacketTypeNormal};
				pq.push_back(pkt);
			}
			REQUIRE( obs1.get_update_count() == v.size() );

		}
		WHEN ( "test observer detach" ) {
			class FooObserver : public Observer
			{
			public:
				FooObserver( const int state ) :
					observer_state( state ),
					update_count(0) {}

				~FooObserver() {}

				int get_state() {
					return observer_state;
				}

				void update( Subject *subject )	{
					update_count++;
				}

				int get_update_count() {
					return update_count;
				}

			private:
				int observer_state;
				int update_count;
			};


			PacketQueue<PacketContent> pq;
			const int N_Observers = 5;
			for (int i=0; i<N_Observers; i++) {
				FooObserver obs(i);
				pq.attach( &obs );
			}
			FooObserver fooObs(5);
			pq.attach( &fooObs );
			REQUIRE( pq.observers.size() == N_Observers + 1 );

			// test detach by pointer
			pq.detach( &fooObs );
			REQUIRE( pq.observers.size() == N_Observers );

			// test detach by index
			pq.detach( 0 );
			pq.detach( 0 );
			REQUIRE( pq.observers.size() == N_Observers - 2 );
		}
	}
}

#if 0
SCENARIO( "Test RFID with block wait" ) {
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug);
        RfidInterface rf;
	bool result;
        GIVEN( "" ) {

                WHEN( "Test Regulation" ) {
			RFID_REGULATION orig_regu;
                        rf.GetRegulation(orig_regu);

                        RFID_REGULATION ut_regu = (RFID_REGULATION) (9 - (int)orig_regu);
			RFID_REGULATION tested_regu;

			result = rf.SetRegulation(ut_regu);
			rf.GetRegulation(tested_regu);

			THEN ( "Verify SetRegulation" ) {
				REQUIRE( ut_regu == tested_regu );
			}

			rf.SetRegulation(orig_regu);
		}

		WHEN( "Test Power" ) {
			// ut means under_test
			int orig_power, ut_power, tested_power;
			int pnResult;
			rf.GetPower(orig_power);
			if (orig_power != 10)
				ut_power = 10;
			else ut_power = 12;
			rf.SetPower(ut_power, &pnResult);
			THEN( "Verify pnResult" ) {
				REQUIRE( pnResult != 0 );
			}
			rf.GetPower(tested_power);
			THEN( "Verify SetPower" ) {
				REQUIRE( (tested_power == ut_power) );
			}
			rf.SetPower(orig_power, &pnResult);
		}

	}
}
#endif


SCENARIO( "Test RFID Interface" ) {
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug);
        RfidInterface rf;

        GIVEN( "" ) {

		WHEN( "Test Reader ID" ) {
			RFID_READER_VERSION ver;
			rf.GetVersion(ver);
                        std::string readerId;
			rf.GetReaderID(readerId);
			THEN ( "Verify Reader ID" ) {
				REQUIRE( ver.strReaderId == readerId );
			}
		}

		WHEN( "Test Loop Antenna" ) {
			unsigned int orig_la = 0;
			unsigned int ut_la = 0x44; // antenna 9, 25
			unsigned int tested_la;
			bool result;
			rf.GetLoopAntenna(orig_la);
			result = rf.SetLoopAntenna(ut_la);
			THEN( "verify command successful" ) {
				REQUIRE( result == true );
			}
			rf.GetLoopAntenna(tested_la);
			THEN( "verify Set Loop Antenna" ) {
				REQUIRE( tested_la == ut_la );
			}
			rf.SetLoopAntenna(orig_la);
		}

		WHEN( "Test Loop Time" ) {
			unsigned int orig_loopTime;
			unsigned int ut_loopTime = 100;
			unsigned int tested_loopTime;
			bool result;
			rf.GetLoopTime(orig_loopTime);
			result = rf.SetLoopTime(ut_loopTime);
			rf.GetLoopTime(tested_loopTime);
			THEN( "verify command successful" ) {
				REQUIRE( result == true );
			}
			THEN( "verify SetLoopTime" ) {
				REQUIRE( tested_loopTime == ut_loopTime );
			}
			rf.SetLoopTime(orig_loopTime);
		}

		WHEN( "Test Time" ) {
			struct tm orig_time;
			struct tm ut_time = { 0, 30, 17, 30, 2, 2021, 2, 89, 1 };
			struct tm tested_time;
			bool result;
			rf.GetTime(orig_time);
			cout << ", sec: "  << orig_time.tm_sec
			     << ", min: "  << orig_time.tm_min
			     << ", hour: " << orig_time.tm_hour
			     << ", month day: "  << orig_time.tm_mday
			     << ", month: "<< orig_time.tm_mon + 1
			     << ", year: " << orig_time.tm_year + 1900
			     << ", week day: " << orig_time.tm_wday
			     << ", year day: " << orig_time.tm_yday
			     << ", dst: " << orig_time.tm_isdst
			     << endl;
			result = rf.SetTime(ut_time);
			rf.GetTime(tested_time);
			cout << ", sec: "  << tested_time.tm_sec
			     << ", min: "  << tested_time.tm_min
			     << ", hour: " << tested_time.tm_hour
			     << ", month day: "  << tested_time.tm_mday
			     << ", month: "<< tested_time.tm_mon + 1
			     << ", year: " << tested_time.tm_year + 1900
			     << ", week day: " << tested_time.tm_wday
			     << ", year day: " << tested_time.tm_yday
			     << ", dst: " << tested_time.tm_isdst
			     << endl;

                        THEN( "verify command successful" ) {
				REQUIRE( result == true );
			}
			THEN( "verify SetTime" ) {
				REQUIRE( tested_time.tm_sec == ut_time.tm_sec );
			}
			rf.SetTime(orig_time);
		}
	}
}
