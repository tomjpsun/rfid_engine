#include <iostream>
#include "parse_ds.hpp"
#include "aixlog.hpp"

using namespace std;

void print_epc(const RFID_TAG_EPC& tag)
{
	LOG(SEVERITY::DEBUG) << COND(DBG_EN)
			     << "strRaw: " << tag.strRaw << ", "
			     << "uiProtocolControl: " << tag.uiProtocolControl << ", "
			     << "strEPC: " << tag.strEPC << ", "
			     << "uiCRC16: " << tag.uiCRC16 << endl;
}

void print_tag(const RFID_TAG_DATA& tag)
{
	LOG(SEVERITY::DEBUG) << COND(DBG_EN)
			     << "uiAntenna: " << tag.uiAntenna << ", "
			     << "strTime: " << tag.strTime << ", "
			     << "strEPC: " << tag.strEPC << ", "
			     << "strTID: " << tag.strTID << ", "
			     << "strUser: " << tag.strUser << endl;
}
