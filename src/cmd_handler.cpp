#include <iostream>
#include <exception>
#include <memory>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <regex>
#include <iomanip>
#include <utility>
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

using namespace rfid;
using namespace std;

CmdHandler::CmdHandler()
{
	ppacket_queue = std::shared_ptr<PacketQueue<PacketContent>>(
		new PacketQueue<PacketContent>);
}


bool CmdHandler::start_recv_thread(string ip_addr, int port_n)
{
	ip = ip_addr;
	port = port_n;
	if (-1 == pipe(notify_pipe)) {
		LOG(SEVERITY::ERROR) << "create pipe error" << endl;
		return false;
	};
	// should not happen, log report
        if ( receive_thread.joinable() ) {
		LOG(SEVERITY::ERROR) << "thread activated twice" << endl;
		return false;
	}

	// construct task_vec with vector's Elem default c'tor,
	// here is a default shared pointer
	// used only in reply_thread_func->recv_callback()
	// since task_vec_index & task_vec is not thread safe
	task_vec_index = 0;
	task_vec.resize(TASK_VEC_MAX_SIZE);

	my_socket = create_socket(ip, port);
	if (!my_socket) {
		LOG(SEVERITY::ERROR) << "create socket failed" << endl;
		return false;
	}
	thread_exit.store(false);
	thread_ready.store(false);
	try {
		receive_thread = std::thread(&CmdHandler::reply_thread_func,
					     this,
					     ip,
					     port);
	} catch ( std::exception &e ) {
		LOG(SEVERITY::ERROR) << "create thread failed" << endl;
		return false;
	}
	// wait for thread ready
	while (!thread_ready)
		this_thread::sleep_for(50ms);

	return true;
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
	// use shutdown + close, refers to:
	// https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
	// the answer by 'Earth Engine'
	shutdown(my_socket, 2);
	close(my_socket);
	// wait for peer shutdown, 50 ms is heuristic value
	std::this_thread::sleep_for(50ms);

	// notify thread to exit
	LOG(TRACE) << COND(LG_RECV) << " notify thread exit" << endl;
	write(notify_pipe[WRITE_ENDPOINT], "a", 1);
	receive_thread.join();
	close(notify_pipe[WRITE_ENDPOINT]);
	close(notify_pipe[READ_ENDPOINT]);

	// clean task threads
	for (auto &p_task : task_vec)
		if (p_task != nullptr && p_task->joinable())
			p_task->join();
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

		while (true) {

			// notify caller that thread is ready
			thread_ready.store(true);

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

			std::string s((char *)buf, n_read);

			recv_callback(s);
		}

		thread_ready.store(false);
		LOG(TRACE) << COND(LG_RECV) <<  "close socket" << endl;
	}
	catch (std::exception& e) {
		thread_ready.store(true);
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

int CmdHandler::send(vector<unsigned char> cmd)
{
	string cmdStr(cmd.begin() + 1, cmd.end() - 1);

	LOG(SEVERITY::DEBUG) << "write(" << cmd.size() << "): " << endl
		   << hex_dump(cmd.data(), cmd.size()) << endl
		   << cmdStr << endl;
	return ::send(my_socket , cmd.data() , cmd.size() , 0 );
}


void CmdHandler::recv_callback(string& in_data)
{
	LOG(SEVERITY::DEBUG) << COND(LG_RECV) << "read (" << in_data.size() << "): " << in_data << endl;

	std::shared_ptr<std::thread> task_thread_ptr =
		std::make_shared<std::thread>(&CmdHandler::task_func,
					      this,
					      in_data);
        task_vec_index = (task_vec_index + 1) % TASK_VEC_MAX_SIZE;
	if (task_vec[task_vec_index] != nullptr &&
	    task_vec[task_vec_index]->joinable()) {
		task_vec[task_vec_index]->join();
		LOG(SEVERITY::NOTICE) << "task_vec full, joining task_vec_index = " << task_vec_index << endl;
	}
        task_vec[task_vec_index] = task_thread_ptr;
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
	while(extract(rgx_boot, PacketTypeHeartBeat) && (++count < MAX_PACKET_EXTRACT_COUNT))
		;
}


bool CmdHandler::extract(const regex rgx, const int ptype)
{
	buffer_mutex.lock();
	// repeatedly match packet pattern
	// sregex_iterator is a template type iterator, which points
	// to the sub-string matched

	auto it = std::sregex_iterator(buffer.begin(), buffer.end(), rgx);
	if ( it == std::sregex_iterator() ) {
		buffer_mutex.unlock();
		return false;
	}
	else {
		std::smatch match = *it;
                PacketContent pkt { match.str(), ptype };
		ppacket_queue->push_back(pkt);
		LOG(SEVERITY::TRACE) << match.str()
				     << ", position:" << match.position()
				     << ", length:" << match.length()
				     << endl;
		buffer.erase(match.position(0), match.length(0));
		buffer_mutex.unlock();

                return true;
	}
}
