#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include "TString.h"

#define LG_RECV  true
#define LG_BUFFER true
#define TCHAR char
#define _T(x) x


std::string hex_dump(void* from, int count);
std::string get_current_dir();
void print_endian();

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


template <typename T>
void unique_add_replace(T& container,
			typename T::reference element,
			std::function<bool (typename T::reference item)> compare)
{
	auto result = std::find_if(std::begin(container),
				   std::end(container),
				   compare);

	bool found = (result != std::end(container));

	if (!found) {
		container.push_back(element);
	}else{
		*result = std::move(element);
	}
}

#endif
