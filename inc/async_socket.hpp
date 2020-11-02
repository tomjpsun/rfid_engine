#ifndef __ASYNC_SOCKET_HPP__
#define __ASYNC_SOCKET_HPP__

#include <memory>
#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace boost;
using p_socket_t = std::shared_ptr<asio::ip::tcp::socket>;
using buffer_t = std::vector<uint8_t>;
using p_buffer_t = std::shared_ptr<buffer_t>;

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
