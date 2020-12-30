#include <iostream>
#include <exception>
#include <memory>
#include <chrono>
#include <asio/asio.hpp>

#include "aixlog.hpp"
#include "cmd_handler.hpp"
#include "common.hpp"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>



using namespace rfid;
using namespace std;


CmdHandler::CmdHandler(string ip_addr, int port_n)
{
	ip = ip_addr;
	port = port_n;
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
	// release thread
	thread_exit.store(true);
	close(my_socket);
	receive_thread.join();
}

void CmdHandler::reply_thread_func(string ip, int port)
{
	unsigned char buf[BUF_SIZE];
	try {
		while (!thread_exit) {
			// notify caller that thread is ready
			thread_ready.store(true);

			int n_read = read(my_socket, buf, BUF_SIZE);
			LOG(TRACE) << "(): read(" << n_read << "): " << endl << hex_dump(buf, n_read) << endl;
			for (int k = 0; k < n_read; k++)
				p_buffer->push_back(buf[k]);
		}
		LOG(TRACE) << "(): close socket" << endl;
	}
	catch (std::exception& e) {
		LOG(TRACE) << "(), exception:" << e.what() << endl;
	}

}

int CmdHandler::create_socket(string ip, int port)
{
	int sock = 0;
	struct sockaddr_in serv_addr;
	//char *hello = "Hello from client";
	//char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return ERROR;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return ERROR;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return ERROR;
	}

//	send(sock , hello , strlen(hello) , 0 );
//	printf("Hello message sent\n");
//	valread = read( sock , buffer, 1024);
//	printf("%s\n",buffer );

	return sock;
}

void CmdHandler::send_cmd(buffer_t cmd)
{
	send(my_socket , cmd.data() , cmd.size() , 0 );
}
