#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <memory>
#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using p_socket_t = std::shared_ptr<boost::asio::ip::tcp::socket>;
using buffer_t = std::vector<uint8_t>;
using p_buffer_t = std::shared_ptr<buffer_t>;

std::string hex_dump(void* from, int count);

template<typename T>
void concat( std::vector<T>& dest, std::vector<T> src );

#endif
