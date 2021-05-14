#include "cpp_if.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <climits>
#include <condition_variable>
#include <vector>
#include <memory>
#include "common.hpp"
#include "cmd_handler.hpp"
#include "packet_content.hpp"
#include "observer.hpp"
#include "aixlog.hpp"
#include "send_sync_observer.hpp"
#include "rfid_if.hpp"
#include "rfid_err.h"
#include "parser.hpp"
#include "nlohmann/json.hpp"

using namespace std;
using namespace rfid;


// holds obj which user had opened, erase on user close it
static vector<shared_ptr<RfidInterface>> handles;
static char json_buffer[JSON_BUFFER_SIZE];

inline void clear_buffer() { ::memset(json_buffer, 0, JSON_BUFFER_SIZE); }




HANDLE RFOpen(PQParams* connection_settings)
{
	shared_ptr<RfidInterface> prf =
		shared_ptr<RfidInterface>(new RfidInterface(*connection_settings));

	int h = INVALID_HANDLE;
	if (handles.size() < MAX_HANDLE_SIZE) {
		handles.push_back(prf);
		h = handles.size() - 1;
	} else {
		// find an empty shared ptr
		for (int j = 0; j < MAX_HANDLE_SIZE; j++) {
			if (handles[j] == nullptr) {
				handles[j] = prf;
				h = j;
			}
		}
	}
	return h;
}

HANDLE RfidOpen(char *ip_addr, char ip_type, int port) {
	PQParams params;
	params.ip_type = ip_type;
	params.port = port;
	snprintf(params.ip_addr, IP_ADDR_LEN, "%s", ip_addr);
	return RFOpen(&params);
}




int RFInventoryEPC(HANDLE h, int slot, bool loop, char **json_str, int* json_len)
{
	int ret;
	vector<string> responses;
	vector<RfidParseU> convert;

	clear_buffer();

        if (!handles[h]) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		shared_ptr<RfidInterface> p = handles[h];
		ret = p->InventoryEPC(slot, loop, responses);
		for (auto& response : responses) {
			convert.push_back( RfidParseU {response} );
		}
		nlohmann::json j = convert;
		string s = j.dump();
		if ( s.size() >= JSON_BUFFER_SIZE )
			ret = RFID_ERR_BUFFER_OVERFLOW;
		else {
			*json_len = snprintf(json_buffer, JSON_BUFFER_SIZE, "%s", s.data());
			*json_str = json_buffer;
			LOG(SEVERITY::TRACE) << "json len = " << *json_len << endl;
		}
	}
        return ret;
}

void RFClose(HANDLE h)
{
	if (handles[h])
		handles[h].reset();
	handles[h] = nullptr;
}
