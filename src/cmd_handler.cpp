#include <iostream>
#include <exception>
#include <memory>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "aixlog.hpp"
#include "cmd_handler.hpp"
#include "common.hpp"

using namespace rfid;
using namespace std;
using namespace boost;


CmdHandler::CmdHandler(string ip_addr, int port_n)
{
	thread_exit.store(false);
	std::promise<int> thread_ready;
	std::future<int> future = thread_ready.get_future();
	ip = ip_addr;
	port = port_n;
	p_buffer = std::make_shared<buffer_t>(0);
	receive_thread = std::thread(&CmdHandler::reply_thread_func,
				     this,
				     ip,
				     port,
				     &thread_ready);
	future.wait();
}

void CmdHandler::reply_thread_func(string ip, int port, promise<int>* thread_ready)
{
	boost::asio::io_service my_io_service;
	asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
	p_socket_t socket(new asio::ip::tcp::socket(my_io_service, ep.protocol()));
	socket->connect(ep);
	p_buffer_t buf;
	int n_read = 0;
	try {
		while (socket->is_open() && !thread_exit) {
			buf = std::make_shared<buffer_t>(128);
			n_read = async_read_socket(socket, buf);
			cout << "read(" << n_read << "): " << endl << hex_dump(buf->data(), n_read) << endl;
			for (int k = 0; k < n_read; k++)
				p_buffer->push_back((*buf)[k]);
		}
		LOG(TRACE) << "(): close socket" << endl;
	}
	catch (std::exception& e) {
		LOG(TRACE) << "(), exception:" << e.what() << endl;
	}

}
