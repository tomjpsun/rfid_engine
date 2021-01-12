#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <memory>
#include <vector>

#include <asio/asio.hpp>

#define LG_RECV  true
#define LG_BUFFER true

std::string hex_dump(void* from, int count);
std::string get_current_dir();

template<typename T>
void concat( std::vector<T>& dest, std::vector<T> src );


// Container split [ *begin, *(end-1) ] from container
template<class C> inline
C split(C& src, typename C::iterator begin, typename C::iterator end)
{
        C subset(begin, end);
	src.erase(begin, end);
	return subset;
}

#endif
