#include <iostream>
#include <exception>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "async_socket.hpp"

using namespace std;
using namespace boost;

void async_socket_read(string from_ip, int port)
{
	const int BUF_SIZE = 20;
	const int TOTAL_DATA_SIZE = 1000;

	boost::asio::io_service my_io_service;
	asio::ip::tcp::endpoint ep(asio::ip::address::from_string(from_ip), port);
	p_socket_t socket(new asio::ip::tcp::socket(my_io_service, ep.protocol()));

	int8_t buffer[BUF_SIZE];
	int data_len = 0;
	try {
		socket->connect(ep);
		while (socket->is_open() &&
			data_len < TOTAL_DATA_SIZE ) {
			data_len += async_read_socket(socket, buffer, BUF_SIZE);
			cout << "read total " << data_len << " bytes from peer" << endl;
		}
		cout << "buffer[" << data_len << "]: " << hex_dump(buffer, data_len) << endl;
		cout << __func__ << "(): close socket" << endl;
	}
	catch (std::exception& e) {
		cout << __func__ << "(), exception:" << e.what() << endl;
	}
}

int main(int argc, char** argv)
{
	async_socket_read("210.242.181.136", 80);
	return 0;
}
