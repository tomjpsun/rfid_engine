#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <sstream>
#include <iomanip>
#include <utility>
#include <catch2/catch.hpp>
#include "buffer.hpp"
#include "aixlog.hpp"
using namespace std;

SCENARIO( "Buffer class test cases", "[Buffer]" ) {
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);

	GIVEN( "initialize Buffer with initializer list" ) {
		Buffer<uint8_t> buffer{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

		WHEN( "use copy constructor" ) {
			Buffer<uint8_t> cp_buffer(buffer);
			THEN ("Size should be equal") {
				REQUIRE( buffer.size() == cp_buffer.size() );
			}
			THEN ("Contents should be equal") {
				bool equal{true};
				for (size_t i=0; i<buffer.size(); i++)
					equal = equal && (buffer[i] == cp_buffer[i]);
				REQUIRE( equal );
			}
		}

		WHEN( "test emplace_back" ) {
			Buffer<uint8_t> extra {16, 17, 18, 19};
			int size_before_concat = buffer.size();
			buffer.emplace_back(extra);
			int size_after_concat = buffer.size();
			THEN ("check concat size") {
				REQUIRE( size_after_concat == size_before_concat + extra.size() );
			}
			THEN ("validate Contents") {
				bool equal{true};
				for (size_t i = 0; i < buffer.size(); i++)
					equal = (buffer[i] == i);
				REQUIRE( equal );
			}
		}
		WHEN( "test Buffer::pop_range()" ) {
			buffer.pop_range(8, 12);
			Buffer<uint8_t> result{0,1,2,3,4,5,6,7, 12,13,14,15};
			THEN ("validate Contents") {
				bool equal{true};
				for (size_t i = 0; i < buffer.size(); i++)
					equal = (buffer[i] == result[i]);
				REQUIRE( equal );
			}
		}

		WHEN( "test Buffer::pop()" ) {
			uint8_t t = buffer.pop();
			t = buffer.pop();
			t = buffer.pop();
			Buffer<uint8_t> result{3,4,5,6,7,8,9,10,11,12,13,14,15};
			THEN ("validate Contents") {
				bool equal{true};
				for (size_t i = 0; i < buffer.size(); i++)
					equal = (buffer[i] == result[i]);
				REQUIRE( equal );
			        REQUIRE( t == 2 );
			}
		}
		WHEN( "test Buffer::find()" ) {

			THEN ("validate non-existed case ") {
				int pos_1 = buffer.find(16);
				REQUIRE( pos_1 == -1 );
			}
			THEN ("validate existed case ") {
				int pos_2 = buffer.find(2);
				REQUIRE( pos_2 == 2 );
			}
			THEN ("validate find pair on ordered series") {
				auto value_pair = std::make_pair(3, 9);
				auto range_pair = buffer.find(value_pair);
				REQUIRE( range_pair.first == 3 );
				REQUIRE( range_pair.second == 9 );
			}
			THEN ("validate find pair on un-ordered series") {
				Buffer<uint8_t> symmetry = {7,6,5,4,3,2,1,0,1,2,3,4,5,6,7};
				auto value_pair = std::make_pair(3, 6);
				auto range_pair = symmetry.find(value_pair);
				REQUIRE(range_pair.first == 4);
				REQUIRE(range_pair.second == 13);
			}
		}
	}
}
