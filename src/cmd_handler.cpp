#include <iostream>
#include <exception>
#include <memory>
#include <chrono>
#include <asio/asio.hpp>

#include "aixlog.hpp"
#include "cmd_handler.hpp"
#include "common.hpp"

using namespace rfid;
using namespace std;


CmdHandler::CmdHandler(string ip_addr, int port_n)
{
	thread_exit.store(false);
	thread_ready.store(false);

	ip = ip_addr;
	port = port_n;
	p_buffer = std::make_shared<buffer_t>(0);
	receive_thread = std::thread(&CmdHandler::reply_thread_func,
				     this,
				     ip,
				     port);
	// wait for thread ready
	while (!thread_ready)
		this_thread::sleep_for(1ms);
}

CmdHandler::~CmdHandler()
{
	// release thread
	thread_exit.store(true);
	p_socket->close();
	p_socket.reset();
	receive_thread.join();
}

void CmdHandler::reply_thread_func(string ip, int port)
{
	asio::io_context my_io_service;
	asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
	p_socket = p_socket_t(new asio::ip::tcp::socket(my_io_service, ep.protocol()));
	p_socket->connect(ep);
	p_buffer_t buf;
	int n_read = 0;

	try {
		while (p_socket->is_open() && !thread_exit) {
			// notify caller that thread is ready
			thread_ready.store(true);
			buf = std::make_shared<buffer_t>(128);
			n_read = async_read_socket(p_socket, buf);
			LOG(TRACE) << "(): read(" << n_read << "): " << endl << hex_dump(buf->data(), n_read) << endl;
			for (int k = 0; k < n_read; k++)
				p_buffer->push_back((*buf)[k]);
		}
		LOG(TRACE) << "(): close socket" << endl;
	}
	catch (std::exception& e) {
		LOG(TRACE) << "(), exception:" << e.what() << endl;
	}

}
