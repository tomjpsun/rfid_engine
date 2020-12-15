#ifndef _ASYNC_SOCKET_HPP_
#define _ASYNC_SOCKET_HPP_

#include <memory>
#include <vector>

#include "asio/asio.hpp"
#include "common.hpp"

using namespace asio;

// struct for async socket operation
struct read_session {
	p_socket_t sock;
	p_buffer_t buf;
	std::size_t total_bytes_read;
	unsigned int buf_size;
};

struct write_session {
	p_socket_t sock;
	p_buffer_t buf;
	std::size_t total_bytes_written;
};

void async_read_from_socket(
	p_socket_t sock,
	p_buffer_t buf,
	int read_size);

int async_read_socket( p_socket_t p_sock, void* buf, int read_size);
int async_read_socket( p_socket_t p_sock, p_buffer_t p_buf);

void async_write_socket( p_socket_t p_sock, void* out_buf, int write_size);


#endif
