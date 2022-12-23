#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <chrono>
#include <iostream>
#include <iomanip>
#include "c_if.h"
#include "TString.h"


#define TCHAR char
#define _T(x) x
//#ifdef DEBUG_ENABLE
//  #define DBG_EN true
//#else
//  #define DBG_EN false
//#endif

#define LOG_TAG TAG(__FUNCTION__)

std::string hex_dump(void* from, int count);
std::string get_current_dir();
void print_endian();
extern bool DBG_EN;

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

string iptostr(const int* ip, int version = 4);

template<typename Clock, typename Duration>
std::ostream &operator<<(std::ostream &stream,
			 const std::chrono::time_point<Clock, Duration> &time_point) {
	const time_t time = Clock::to_time_t(time_point);
#if __GNUC__ > 4 ||							\
	((__GNUC__ == 4) && __GNUC_MINOR__ > 8 && __GNUC_REVISION__ > 1)
	// Maybe the put_time will be implemented later?
	struct tm tm;
	localtime_r(&time, &tm);
	return stream << std::put_time(&tm, "%c"); // Print standard date&time
#else
	char buffer[26];
	ctime_r(&time, buffer);
	buffer[24] = '\0';  // Removes the newline that is added
	return stream << buffer;
#endif
}

// for debug purpose
inline void print_timepoint(std::chrono::system_clock::time_point timepoint) {
	std::cout << timepoint << std::endl;
}
inline void print_time_t(time_t epoch)
{
	printf("%s", asctime(gmtime(&epoch)));
}



#endif
