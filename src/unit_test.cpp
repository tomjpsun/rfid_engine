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

SCENARIO( "Test RFID Interface" ) {
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::debug);
        RfidInterface rf;
        GIVEN( "" ) {
		WHEN( "test RFID API" ) {
			bool result;

			RFID_READER_VERSION ver;
			rf.GetVersion(ver);
			cout << "fw: " << ver.strFirmware
			     << ", hw: " << ver.strHardware
			     << ", id: " << ver.strReaderId
			     << ", band regulation: " << ver.strRfBandRegualation
			     << endl;

                        std::string readerId;
			result = rf.GetReaderID(readerId);
			cout << "result: " << result << ", readerId: " << readerId << endl;
			THEN (" Reader ID should be the same ") {
				REQUIRE( ver.strReaderId == readerId );
			}

			RFID_REGULATION regu;
//			rf.SetRegulation(REGULATION_US);
			rf.GetRegulation(regu);
			cout << "regulation: " << int(regu) << endl;

			int power;
			rf.GetPower(power);
			cout << "power: " << power << endl;

			unsigned int antenna = 0;
			bool hub = false;
			rf.GetSingleAntenna(antenna, hub);
			cout << "antenna: " << antenna << ", hub: " << hub << endl;

			unsigned int loopAntenna = 0;
			rf.GetLoopAntenna(loopAntenna);
			cout << "loop antenna: " << loopAntenna << endl;

			unsigned int loopTime;
			rf.GetLoopTime(loopTime);
			cout << "loop time: " << loopTime << endl;

			struct tm time;
			result = rf.GetTime(time);
			cout << "result: " << result
			     << ", sec: "  << time.tm_sec
			     << ", min: "  << time.tm_min
			     << ", hour: " << time.tm_hour
			     << ", month day: "  << time.tm_mday
			     << ", month: "<< time.tm_mon + 1
			     << ", year: " << time.tm_year + 1900
			     << ", week day: " << time.tm_wday
			     << ", year day: " << time.tm_yday
			     << ", dst: " << time.tm_isdst
			     << endl;

		}
	}
}
