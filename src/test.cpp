#include <iostream>
#include <exception>

#include "async_socket.hpp"
#include "aixlog.hpp"
#include "common.hpp"
#include "cmd_handler.hpp"

using namespace std;
using namespace rfid;


int main(int argc, char** argv)
{
	AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
	CmdHandler cmdHandler("192.168.88.91", 1001);
	vector<uint8_t> cmd{0x0A, 0x40, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x0D};
	cmdHandler.send_cmd(cmd);

//	asio::io_context io_context;
//	asio::ip::tcp::endpoint ep(asio::ip::address::from_string("192.168.88.91"), 1001);
//	p_socket_t socket(new asio::ip::tcp::socket(io_context));
//	socket->connect(ep);
//
//	vector<uint8_t> cmd{0x0A, 0x40, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x0D};
//	cout << "write(" << cmd.size() << "): " << endl << hex_dump(cmd.data(), cmd.size()) << endl;
//	async_write_socket(socket, cmd.data(), cmd.size());
//	async_socket_read(socket);
//
//	socket->shutdown(asio::ip::tcp::socket::shutdown_send);
	return 0;
}
