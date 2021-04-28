#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

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

// trim from start (in place)
static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

#endif
