#include <iostream>
#include <exception>
#include <chrono>
#include <thread>
#include <algorithm>
#include <chrono>
#include "aixlog.hpp"
#include "common.hpp"
#include "cmd_handler.hpp"
#include "cpp_if.hpp"
#include "conn_queue.hpp"
#include "rfid_config.hpp"
#include "rfid_if.hpp"
#include "parser.hpp"
#include "c_if.h"


#define TEST_REBOOT


using namespace std;
using namespace rfid;

extern RfidConfig g_cfg;
int cpp_test(ReaderSettings* rs)
{
	RfidConfig cfg = RfidConfigFactory().get_config();

	RfidInterface rf(*rs);

	int ret;
	vector<string> read_mb;

	RFID_READER_VERSION ver;
	ret = rf.GetVersion(ver);
	cout << "return: " << ret
	     << ", fw: " << ver.strFirmware
	     << ", hw: " << ver.strHardware
	     << ", id: " << ver.strReaderId
	     << ", band regulation: " << ver.strRfBandRegualation
	     << endl;


#ifdef TEST_REBOOT
        cout << "start Reboot()" << flush << endl;
	rf.Reboot();
        cout << "finish Reboot()" << flush << endl;
#endif

	ret = rf.GetVersion(ver);
	cout << "return: " << ret
	     << ", fw: " << ver.strFirmware
	     << ", hw: " << ver.strHardware
	     << ", id: " << ver.strReaderId
	     << ", band regulation: " << ver.strRfBandRegualation
	     << endl;

        return 0;
}


int main(int argc, char** argv)
{
	int ret;
	ReaderSettings rs[2] = {
		{
			ReaderSettingsType(SOCKET), // socket type
			{192, 168, 88, 94 }, // ipv4
			{}, // ipv6
			{}, // dev name
			1001, // port
			28, // power
			40, // loop time
			1 // antenna id
		},
		{
			ReaderSettingsType(SOCKET), // socket type
			{192, 168, 88, 91 }, // ipv4
			{}, // ipv6
			{}, // dev name
			1001, // port
			28, // power
			40, // loop time
			1 // antenna id
		}
	};

        if ( (ret = RFModuleInit()) != RFID_OK ) {
		return ret;
	}

        int device_index = 0;
        return cpp_test(rs);

}
