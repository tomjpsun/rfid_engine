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
#include "handle_manager.hpp"

using namespace std;
using namespace rfid;


// holds obj which user had opened, erase on user close it
static HandleManager hm{};

//static char json_buffer[JSON_BUFFER_SIZE];

HANDLE RFOpen(PQParams* connection_settings)
{
	shared_ptr<RfidInterface> prf =
		shared_ptr<RfidInterface>(new RfidInterface(*connection_settings));
	return hm.add_handle_unit(prf);
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

        if ( !hm.is_valid_handle(h) ) {
		ret = RFID_ERR_INVALID_HANDLE;
	} else {
		hm.get_rfid_ptr(h)->InventoryEPC(slot, loop, responses);
		for (auto& response : responses) {
			convert.push_back( RfidParseU {response} );
		}
		nlohmann::json j = convert;
		string s = j.dump();

		hm.clear_buffer(h);
		hm.append_data(h, s);
		*json_str = hm.get_data(h, json_len);
		LOG(SEVERITY::TRACE) << "json str = " << *json_str << endl;
	}
        return ret;
}

void RFClose(HANDLE h)
{
	hm.remove_handle_unit(h);
}
