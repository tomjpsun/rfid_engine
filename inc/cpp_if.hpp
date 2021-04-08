#ifndef _CPP_IF_HPP_
#define _CPP_IF_HPP_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <climits>
#include "common.hpp"
#include "cmd_handler.hpp"
#include "packet_content.hpp"
#include "observer.hpp"
#include "aixlog.hpp"

using namespace std;
using namespace rfid;

extern "C"
{

#define IP_ADDR_LEN 16
#define IP_TYPE_IPV4 1
#define IP_TYPE_IPV6 2
#define MAX_PACKET_EXTRACT_COUNT 20
#define DEFAULT_SET_POWER_VALUE  10
#define RF_MODULE_RESET_TIME_MS  3000

        typedef struct _PQParams_ {
		char ip_addr[IP_ADDR_LEN];
		char ip_type; // IP_TYPE_IPV(4|6)
		int port; // default 1001
		int loop; // default 100
	}PQParams, *PPQParams;
}

#endif // _CPP_IF_HPP_
