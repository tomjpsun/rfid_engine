#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <cstring>
#include <memory>
#include "aixlog.hpp"

#include "async_socket.hpp"
#include "common.hpp"

using namespace std;
using namespace asio;


////////////////////
// async read socket
////////////////////

#if BOOST_VERSION >= 107000
#define GET_IO_SERVICE(p_sock) ((io_context&)(p_sock->get_executor().context()))
#else
#define GET_IO_SERVICE(p_sock) (p_sock->get_io_service())
#endif



void async_read_callback(const boost::system::error_code& ec,
		    std::size_t bytes_transferred,
		    std::shared_ptr<read_session> s)
{
	LOG(TRACE) << __func__ << "(), read " << bytes_transferred << " bytes" << endl;

	if (ec.value() !=0) {
		LOG(TRACE) <<  __func__
			   << "() capture exception: code = "
			   << ec.value()
			   << ". Message: "
			   << ec.message()
			   << endl;
		//throw std::runtime_error(ec.message());
		return;
	}
	s->total_bytes_read += bytes_transferred;
	if (s->total_bytes_read == s->buf_size) {
		LOG(TRACE) << __func__
			   << "(), exit! total read "
			   << s->buf_size
			   << " bytes"
			   << endl;
		//hex_dump(s->buf->data(), 5);
		(GET_IO_SERVICE(s->sock)).stop();
		return;
	}
	s->sock->async_read_some(
		asio::buffer(s->buf->data() + s->total_bytes_read,
			     s->buf_size - s->total_bytes_read),
		std::bind(async_read_callback, std::placeholders::_1,
			  std::placeholders::_2, s));
}

void async_read_from_socket_x(
	p_socket_t sock,
	p_buffer_t buf,
	int read_size)
{
	std::shared_ptr<read_session> s(new read_session);
// Allocating the buffer.
	s->buf = buf;//.reset(new buffer_t(read_size));
	s->total_bytes_read = 0;
	s->sock = sock;
	s->buf_size = read_size;
// Initiating asynchronous reading operation.
	s->sock->async_read_some(
		asio::buffer(s->buf->data(), s->buf_size),
		std::bind(async_read_callback,
			  std::placeholders::_1,
			  std::placeholders::_2,
			  s));
	LOG(TRACE) <<  "leave " << __func__ << "()" << endl;

}

void async_read_from_socket(
	p_socket_t sock,
	p_buffer_t buf,
	int read_size)
{
	LOG(TRACE) << __func__
		   << "(), request for "
		   << read_size
		   << " bytes"
		   << endl;

	boost::asio::io_service& ios = GET_IO_SERVICE(sock);
	async_read_from_socket_x(sock, buf, read_size);
	LOG(TRACE) << "before ioservice.run()" << endl;
	ios.run();
	LOG(TRACE) << "after ioservice.run()" << endl;
	// prepare for next run()
	ios.reset();
}

int async_read_socket( p_socket_t p_sock, p_buffer_t p_buf)
{
	async_read_from_socket(p_sock, p_buf, p_buf->size());
	return p_buf->size();
}

int async_read_socket( p_socket_t p_sock, void* in_buf, int read_size)
{
	uint8_t *buf = (uint8_t *)in_buf;
	p_buffer_t p_buf = p_buffer_t(new buffer_t(read_size));
	async_read_from_socket(p_sock, p_buf, read_size);
#ifndef _WINDOWS
	//(+) 2019-02-25 Richard Chung
	// original
	std::copy(p_buf->data(), p_buf->data() + p_buf->size(), buf);
	// Modified
#else
	memcpy_s(buf, read_size, p_buf->data(), p_buf->size());
#endif
	//(-) 2019-02-25 Richard Chung
	return p_buf->size();
}



////////////////////
// async write socket
////////////////////



void async_write_callback(const boost::system::error_code& ec,
		    std::size_t bytes_transferred,
		    std::shared_ptr<write_session> s)
{
	LOG(TRACE) <<  __func__ << "(), write " << bytes_transferred << " bytes" << endl;

	if (ec.value() !=0) {
		LOG(TRACE) << __func__
			   << "() capture exception: code = "
			   << ec.value()
			   << ". Message: "
			   << ec.message()
			   << endl;
		//throw std::runtime_error(ec.message());
		return;
	}
	s->total_bytes_written += bytes_transferred;
	if (s->total_bytes_written == s->buf->size()) {
		LOG(TRACE) << __func__
			   << "(), exit! total written "
			   << s->buf->size()
			   << " bytes"
			   << endl;
		//hex_dump(s->buf->data(), 5);
		boost::asio::io_service& ios = GET_IO_SERVICE(s->sock);
		ios.stop();
		return;
	}
	s->sock->async_write_some(
		asio::buffer(s->buf->data() + s->total_bytes_written,
			     s->buf->size() - s->total_bytes_written),
		std::bind(async_write_callback, std::placeholders::_1,
			  std::placeholders::_2, s));
}


void async_write_to_socket_x(
	p_socket_t sock,
	p_buffer_t buf)
{
	std::shared_ptr<write_session> s(new write_session);
// Allocating the buffer.
	s->buf = buf;
	s->total_bytes_written = 0;
	s->sock = sock;

// Initiating asynchronous reading operation.
	s->sock->async_write_some(
		asio::buffer(s->buf->data(), s->buf->size()),
		std::bind(async_write_callback,
			  std::placeholders::_1,
			  std::placeholders::_2,
			  s));
	LOG(TRACE) <<  "leave " << __func__ << "()" << endl;

}

void async_write_to_socket(
	p_socket_t sock,
	p_buffer_t buf)
{
	LOG(TRACE) << __func__
		   << "(), write "
		   << buf->size()
		   << " bytes"
		   << endl;

	boost::asio::io_service& ios = GET_IO_SERVICE(sock);
	async_write_to_socket_x(sock, buf);
	LOG(TRACE) << "before ioservice.run()" << endl;
	ios.run();
	LOG(TRACE) << "after ioservice.run()" << endl;
	// prepare for next run()
	ios.reset();
}

void async_write_socket( p_socket_t p_sock, void* out_buf, int write_size)
{
	uint8_t *buf = (uint8_t *)out_buf;
	p_buffer_t p_buf = p_buffer_t(new buffer_t(buf, buf + write_size));
	return async_write_to_socket(p_sock, p_buf);
}
