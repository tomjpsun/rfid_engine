#include <iostream>
#include <exception>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "async_socket.hpp"
#include "logger.hpp"
#include "utils.hpp"
using namespace std;
using namespace boost;
using namespace boost::log::trivial;
static src::severity_logger< severity_level > g_lg;

void async_socket_read(p_socket_t socket)
{
	const int BUF_SIZE = 20;

	int8_t buffer[BUF_SIZE];
	int n_read = 0;
	try {
		while (socket->is_open() ) {
			n_read = async_read_socket(socket, buffer, BUF_SIZE);
			cout << "reae(" << n_read << "): " << endl << hex_dump(buffer, n_read) << endl;
		}
		cout << __func__ << "(): close socket" << endl;
	}
	catch (std::exception& e) {
		cout << __func__ << "(), exception:" << e.what() << endl;
	}
}

int main(int argc, char** argv)
{
	init_logging("rfid_test.log", 1);
	boost::asio::io_service my_io_service;
	asio::ip::tcp::endpoint ep(asio::ip::address::from_string("192.168.88.91"), 1001);
	p_socket_t socket(new asio::ip::tcp::socket(my_io_service, ep.protocol()));
	socket->connect(ep);

	vector<uint8_t> cmd{0x0A, 0x40, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x0D};
	cout << "write(" << cmd.size() << "): " << endl << hex_dump(cmd.data(), cmd.size()) << endl;
	async_write_socket(socket, cmd.data(), cmd.size());
	async_socket_read(socket);

	socket->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
	return 0;
}
