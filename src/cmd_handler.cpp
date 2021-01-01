#include <iostream>
#include <exception>
#include <memory>
#include <chrono>
#include <cstring>
#include <algorithm>
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
	LOG(TRACE) << " destructor, set thread exit flag" << endl;
	// release thread
	thread_exit.store(true);
	LOG(TRACE) << " close socket" << endl;
	close(my_socket);
	LOG(TRACE) << " notify thread exit" << endl;
	write(notify_pipe[WRITE_END], "a", 1);
	receive_thread.join();
	close(notify_pipe[WRITE_END]);
	close(notify_pipe[READ_END]);
}

void CmdHandler::set_poll_fd(struct pollfd* p_poll_fd)
{
	struct pollfd default_poll_fd[2] = {
		{ .fd = my_socket,
		  .events = POLLIN },
		{ .fd = notify_pipe[READ_END],
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

		while (--loop > 0) {

			set_poll_fd(poll_fd);

			// block on 2 fds until either socket or notify_pipe[READ_END] has input data
			// -1 : no timeout, wait forever
			poll(poll_fd, 2, -1);

			// if notify_pipe[READ_END] has input data, leave thread
			if (poll_fd[1].revents & POLLIN)
				break;

			// by man page of read:  if message size larger than the requested,
			// the last one char may be dropped, we use BUF_SIZE - 1 to
			// prevent it from happen

			// peek bytes available, not moving data yet
			ssize_t n_read = recv(my_socket, buf, BUF_SIZE -1, MSG_PEEK);
			// real receive
			std::memset(buf, 0, BUF_SIZE);
			n_read = recv(my_socket,
				      buf,
				      min(n_read, ssize_t(BUF_SIZE - 1)),
				      0);

			LOG(DEBUG) << "read(" << n_read << "): " << endl << hex_dump(buf, n_read) << endl;
			//for (int k = 0; k < n_read; k++)
			//	p_buffer->push_back(buf[k]);
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

void CmdHandler::send(buffer_t cmd)
{
	LOG(TRACE) << "write(" << cmd.size() << "): " << endl << hex_dump(cmd.data(), cmd.size()) << endl;
	::send(my_socket , cmd.data() , cmd.size() , 0 );
}
