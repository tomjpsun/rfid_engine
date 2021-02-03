#include <iostream>
#include <exception>
#include <memory>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <regex>
#include <iomanip>
#include <asio/asio.hpp>

#include "aixlog.hpp"
#include "cmd_handler.hpp"
#include "common.hpp"
#include "cpp_intf.hpp"

#include <stdio.h>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

using namespace rfid;
using namespace std;

CmdHandler::CmdHandler()
{
}


void CmdHandler::start_recv_thread(string ip_addr, int port_n, int loop_count)
{
	loop = loop_count;
	ip = ip_addr;
	port = port_n;
	if (-1 == pipe(notify_pipe)) {
		LOG(ERROR) << "create pipe error" << endl;
		exit(ERROR);
	};
	my_socket = create_socket(ip, port);
	thread_exit.store(false);
	thread_ready.store(false);

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
}

void CmdHandler::stop_recv_thread()
{
	LOG(TRACE) << COND(LG_RECV) << " set thread exit flag" << endl;
	// release thread
	thread_exit.store(true);
	LOG(TRACE) << COND(LG_RECV) << " close socket" << endl;
	close(my_socket);
	LOG(TRACE) << COND(LG_RECV) << " notify thread exit" << endl;
	write(notify_pipe[WRITE_ENDPOINT], "a", 1);
	receive_thread.join();
	close(notify_pipe[WRITE_ENDPOINT]);
	close(notify_pipe[READ_ENDPOINT]);

}

void CmdHandler::set_poll_fd(struct pollfd* p_poll_fd)
{
	struct pollfd default_poll_fd[2] = {
		{ .fd = my_socket,
		  .events = POLLIN },
		{ .fd = notify_pipe[READ_ENDPOINT],
		  .events = POLLIN }
	};
	memcpy((void*)p_poll_fd, default_poll_fd, sizeof(struct pollfd) * 2);

}

void CmdHandler::reply_thread_func(string ip, int port)
{
	unsigned char buf[BUF_SIZE];
	struct pollfd poll_fd[2];

	try {
		// notify caller that thread is ready
		thread_ready.store(true);

		while (loop-- > 0) {

			set_poll_fd(poll_fd);

			// block on 2 fds until either socket or notify_pipe[READ_ENDPOINT] has input data
			// -1 : no timeout, wait forever
			poll(poll_fd, 2, -1);

			// if notify_pipe[READ_ENDPOINT] has input data, leave thread
			if (poll_fd[1].revents & POLLIN)
				break;

			// peek bytes available, not moving data yet
			ssize_t n_read = recv(my_socket, buf, BUF_SIZE, MSG_PEEK);
			// real receive
			std::memset(buf, 0, BUF_SIZE);
			n_read = recv(my_socket,
				      buf,
				      n_read,
				      0);

			LOG(TRACE) << COND(LG_RECV) << "loop " << loop
				   << ": read(" << n_read << "): " << endl
				   << hex_dump(buf, n_read) << endl;

			std::string s((char *)buf, n_read);

			recv_callback(s);
		}
		LOG(TRACE) << COND(LG_RECV) <<  "close socket" << endl;
	}
	catch (std::exception& e) {
		LOG(TRACE) << "(), exception:" << e.what() << endl;
	}

}

int CmdHandler::create_socket(string ip, int port)
{
	int sock = 0;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		LOG(TRACE) << COND(LG_RECV) << "Socket creation error" << endl;
		return ERROR;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0)
	{
		LOG(TRACE) << COND(LG_RECV) << "Invalid address/ Address not supported"<< endl;
		return ERROR;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		LOG(TRACE) << COND(LG_RECV) << "Connection Failed" << endl;
		return ERROR;
	}

	return sock;
}

void CmdHandler::send(vector<unsigned char> cmd)
{
	LOG(DEBUG) << COND(LG_RECV) << "write(" << cmd.size() << "): " << endl << hex_dump(cmd.data(), cmd.size()) << endl;
	::send(my_socket , cmd.data() , cmd.size() , 0 );
}


void CmdHandler::recv_callback(string& in_data)
{
	LOG(TRACE) << COND(LG_RECV) << "read (" << in_data.size() << "): " << in_data << endl;

	std::thread t(&CmdHandler::process_buffer_thread_func,
		      this,
		      in_data);
	t.detach();
}


void CmdHandler::process_buffer_thread_func(string in_data)
{
	buffer_mutex.lock();
	buffer.append(in_data);
	buffer_mutex.unlock();
	LOG(DEBUG) << COND(LG_RECV) << ": read(" << in_data.size() << "): " << endl
		   << hex_dump( (void*)in_data.data(), in_data.size()) << endl;

	const std::regex rgx( "(\x0a.*\x0d\x0a)" );
	extract(rgx);
	const std::regex rgx_hb( "(3heartbeat\\d{2}-\\d{2}-\\d{2})" );
	extract(rgx_hb);
}


void CmdHandler::extract(const regex rgx)
{
	buffer_mutex.lock();
	// repeatedly match packet pattern
	// sregex_iterator is a template type iterator, which points
	// to the sub-string matched
	deque<string> temp_queue;
	for(auto it = std::sregex_iterator(buffer.begin(), buffer.end(), rgx);
	    it != std::sregex_iterator();
	    ++it) {
		std::smatch match = *it;
		temp_queue.push_back(match.str());
		LOG(TRACE) << COND(LG_RECV) << match.str() << endl;
	}

	buffer_mutex.unlock();

	// append result to packet queue
	for (auto &p: temp_queue) {
		packet_queue.push_back(p);
	}

}
