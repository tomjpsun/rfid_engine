#include <iostream>
#include <exception>
#include <memory>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <regex>
#include <iomanip>
#include <utility>
#include <functional>
#include <asio/asio.hpp>

#include "aixlog.hpp"
#include "cmd_handler.hpp"
#include "common.hpp"
#include "cpp_if.hpp"

#include <stdio.h>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define USE_SERIAL

using namespace rfid;
using namespace std;



CmdHandler::CmdHandler()
{
	ppacket_queue = std::shared_ptr<PacketQueue<PacketContent>>(
		new PacketQueue<PacketContent>);
	reset_heartbeat_callback();
}


bool CmdHandler::start_recv_thread(string ip_addr, int port_n)
{
	ip = ip_addr;
	port = port_n;
	asio::error_code ec;
        // should not happen, log report
        if ( receive_thread.joinable() ) {
		LOG(SEVERITY::ERROR) << "thread activated twice" << endl;
		return false;
	}
	thread_ready.store(false);
	try {
#ifndef USE_SERIAL
		receive_thread = std::thread(&CmdHandler::reply_thread_func,
					     this,
					     ip,
					     port, &ec);
#else
		receive_thread = std::thread(&CmdHandler::reply_thread_func_serial,
					     this,
					     "/dev/ttyUSB0",
					     &ec);
#endif
	} catch ( std::exception &e ) {
		LOG(SEVERITY::ERROR) << "create thread failed" << endl;
		return false;
	}
	// wait for thread ready
	while (!thread_ready)
		this_thread::sleep_for(50ms);

	if (ec.value()) {
		LOG(SEVERITY::ERROR) << ec.message() << endl;
	}
	return true;
}

CmdHandler::~CmdHandler()
{
}

void CmdHandler::stop_recv_thread()
{
	LOG(TRACE) << COND(LG_RECV) << " close socket" << endl;

        // use shutdown + close, refers to:
	// https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
	// the answer by 'Earth Engine'

        LOG(TRACE) << " cancel socket" << endl;
        asio_socket->cancel();
	asio_socket->close();

	receive_thread.join();
}


void CmdHandler::async_read_callback_serial(const asio::error_code& ec,
					    std::size_t bytes_transferred,
					    p_serial_t p_serial)
{
	if (ec.value() != 0) {
		// NOTICE: If the command is 'reboot', its OK to have this error,
		// otherwise, it means networking error happened
		LOG(SEVERITY::NOTICE) << ec.message() << endl;
		return;
	} else {
		std::string s((char *)receive_buffer, bytes_transferred);
		recv_callback(s);
		std::memset(receive_buffer, 0, BUF_SIZE);
		p_serial->async_read_some(
			asio::buffer(receive_buffer, BUF_SIZE),
			std::bind(&CmdHandler::async_read_callback_serial,
				  this,
				  std::placeholders::_1,
				  std::placeholders::_2,
				  p_serial));
	}
}


void CmdHandler::async_read_callback_socket(const asio::error_code& ec,
				     std::size_t bytes_transferred,
				     p_socket_t p_socket)
{
	if (ec.value() != 0) {
		// NOTICE: If the command is 'reboot', its OK to have this error,
		// otherwise, it means networking error happened
		LOG(SEVERITY::NOTICE) << ec.message() << endl;
		return;
	} else {
		std::string s((char *)receive_buffer, bytes_transferred);
		recv_callback(s);
		std::memset(receive_buffer, 0, BUF_SIZE);
		p_socket->async_read_some(
			asio::buffer(receive_buffer, BUF_SIZE),
			std::bind(&CmdHandler::async_read_callback_socket,
				  this,
				  std::placeholders::_1,
				  std::placeholders::_2,
				  p_socket));
	}

}


void CmdHandler::reply_thread_func_serial(string serial_name, asio::error_code* ec_ptr)
{
	asio::io_service io_service;
	asio_serial = std::make_shared<asio::serial_port>( asio::serial_port{ io_service } );
	asio_serial->open(serial_name, *ec_ptr);
	thread_ready.store(true);
	if (ec_ptr->value())
		return;
	// clean buffer before use it
	std::memset(receive_buffer, 0, BUF_SIZE);
	asio_socket->async_read_some(
		asio::buffer(receive_buffer, BUF_SIZE),
		std::bind(&CmdHandler::async_read_callback_serial,
			  this,
			  std::placeholders::_1,
			  std::placeholders::_2,
			  asio_serial));

	io_service.run();
}


void CmdHandler::reply_thread_func(string ip, int port, asio::error_code* ec_ptr)
{
	asio::io_service io_service;
	asio_socket = std::make_shared<asio::ip::tcp::socket>( asio::ip::tcp::socket{io_service} );
	asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
	asio_socket->connect(ep, *ec_ptr);
	thread_ready.store(true);
	if (ec_ptr->value())
		return;
	// clean buffer before use it
	std::memset(receive_buffer, 0, BUF_SIZE);
	asio_socket->async_read_some(
		asio::buffer(receive_buffer, BUF_SIZE),
		std::bind(&CmdHandler::async_read_callback_socket,
			  this,
			  std::placeholders::_1,
			  std::placeholders::_2,
			  asio_socket));

	io_service.run();
}



int CmdHandler::send(vector<unsigned char> cmd)
{
	string cmdStr(cmd.begin() + 1, cmd.end() - 1);
	int nSend;
	LOG(SEVERITY::DEBUG) << "write(" << cmd.size() << "): " << endl
		   << hex_dump(cmd.data(), cmd.size()) << endl
		   << cmdStr << endl;
#ifndef USE_SERIAL
	nSend = asio_socket->send(asio::buffer(cmd.data(), cmd.size()));
#else
	nSend = asio_serial->write_some(asio::buffer(cmd.data(), cmd.size()));
#endif
        return nSend;
}


void CmdHandler::recv_callback(string& in_data)
{
	LOG(SEVERITY::DEBUG) << COND(LG_RECV) << "read (" << in_data.size() << "): " << in_data << endl;
	task_func(in_data);
}


void CmdHandler::task_func(string in_data)
{
	buffer_mutex.lock();
	buffer.append(in_data);
	buffer_mutex.unlock();
	LOG(SEVERITY::TRACE) << COND(LG_RECV) << ": read(" << in_data.size() << "): "
			     << hex_dump( (void*)in_data.data(), in_data.size()) << endl;

	const std::regex rgx( "(\x0a.*\x0d\x0a)" );
	int count = 0;
	while(extract(rgx, PacketTypeNormal) && (++count < MAX_PACKET_EXTRACT_COUNT))
		;
	const std::regex rgx_hb( "([0-9a-fA-F]+heartbeat..-..-..)" );
	count = 0;
	while(extract(rgx_hb, PacketTypeHeartBeat) && (++count < MAX_PACKET_EXTRACT_COUNT))
		;
	const std::regex rgx_boot( "(\x02\x41\x0d)" );
	count = 0;
	while(extract(rgx_boot, PacketTypeReboot) && (++count < MAX_PACKET_EXTRACT_COUNT))
		;
}


bool CmdHandler::extract(const regex rgx, const int ptype)
{
	lock_guard<std::mutex> lock(buffer_mutex);
	// repeatedly match packet pattern
	// sregex_iterator is a template type iterator, which points
	// to the sub-string matched

	auto it = std::sregex_iterator(buffer.begin(), buffer.end(), rgx);
	if ( it == std::sregex_iterator() ) {
		return false;
	}
	else {
		std::smatch match = *it;
                PacketContent pkt { match.str(), ptype };
		switch(ptype) {
			case PacketTypeReboot:
			case PacketTypeNormal:
			{
				ppacket_queue->push_back(pkt);
				LOG(SEVERITY::TRACE) << match.str()
						     << ", position:" << match.position()
						     << ", length:" << match.length()
						     << endl;
			}
			break;

			case PacketTypeHeartBeat:
			{
				if (heartbeat_callback_function != nullptr) {
					heartbeat_callback_function(pkt.to_string());
				} else {
					LOG(SEVERITY::NOTICE) << "get heartbeat but null callback func, input = "
							      << pkt.to_string() << endl;
				}
			}
			break;
		}
		buffer.erase( match.position(0), match.length(0) );
                return true;
	}
}
