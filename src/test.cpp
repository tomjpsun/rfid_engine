#include <iostream>
#include <exception>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "async_socket.hpp"

using namespace std;
using namespace boost;

void async_socket_read(p_socket_t socket)
{
	const int BUF_SIZE = 20;
	const int TOTAL_DATA_SIZE = 1000;

	int8_t buffer[BUF_SIZE];
	int data_len = 0;
	try {
		while (socket->is_open() &&
			data_len < TOTAL_DATA_SIZE ) {
			data_len += async_read_socket(socket, buffer, BUF_SIZE);
			cout << "buffer[" << data_len << "]: " << endl << hex_dump(buffer, data_len) << endl;
		}
		cout << __func__ << "(): close socket" << endl;
	}
	catch (std::exception& e) {
		cout << __func__ << "(), exception:" << e.what() << endl;
	}
}

int main(int argc, char** argv)
{
	boost::asio::io_service my_io_service;
	asio::ip::tcp::endpoint ep(asio::ip::address::from_string("192.168.88.91"), 1001);
	p_socket_t socket(new asio::ip::tcp::socket(my_io_service, ep.protocol()));
	socket->connect(ep);

	vector<uint8_t> cmd{0x0A, 0x40, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x0D};
	async_write_socket(socket, cmd.data(), cmd.size());
	async_socket_read(socket);

	socket->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
	return 0;
}
