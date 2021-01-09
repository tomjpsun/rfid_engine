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

#include <stdio.h>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

using namespace rfid;
using namespace std;


CmdHandler::CmdHandler(string ip_addr, int port_n, int loop_count)
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
	LOG(TRACE) << " destructor, set thread exit flag" << endl;
	// release thread
	thread_exit.store(true);
	LOG(TRACE) << " close socket" << endl;
	close(my_socket);
	LOG(TRACE) << " notify thread exit" << endl;
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

			//LOG(DEBUG) << "loop " << loop << ": read(" << n_read << "): " << endl << hex_dump(buf, n_read) << endl;
			std::string s((char *)buf, n_read);
			LOG(DEBUG) << "loop " << loop <<  endl;
			recv_callback(s);
		}
		LOG(TRACE) << "close socket" << endl;
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
		LOG(TRACE) << "Socket creation error" << endl;
		return ERROR;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0)
	{
		LOG(TRACE) << "Invalid address/ Address not supported"<< endl;
		return ERROR;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		LOG(TRACE) << "Connection Failed" << endl;
		return ERROR;
	}

	return sock;
}

void CmdHandler::send(vector<unsigned char> cmd)
{
	LOG(TRACE) << "write(" << cmd.size() << "): " << endl << hex_dump(cmd.data(), cmd.size()) << endl;
	::send(my_socket , cmd.data() , cmd.size() , 0 );
}


void CmdHandler::recv_callback(string& in_data)
{
	cout << "read (" << in_data.size() << "): " << in_data << endl;

	std::thread thrd = std::thread(&CmdHandler::process_buffer_thread_func,
				       this,
				       in_data);

}

void CmdHandler::process_buffer_thread_func(string in_data)
{
	std::lock_guard<std::mutex> lock(buffer_mutex);
	buffer.append(in_data);

	const std::regex re( "\\r.*\\n");
	std::smatch match;
	if( std::regex_match( in_data, match, re ) ) {
		std::cout << "start: at " << match.position(1) << " found " << std::quoted( match[1].str() ) << endl
			  << "  end: at " << match.position(3) << " found " << std::quoted( match[3].str() ) << endl
			  << "sequence between start and stop: " << std::quoted( match[2].str() ) << endl;
	}

}
