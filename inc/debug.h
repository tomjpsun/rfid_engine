#ifndef __debug_h__
#define __debug_h__

#include <iostream>

struct X {
#ifdef DEBUG
	~X() { std::cout << std::endl; }
#else
	~X() { std::cout << std::endl; std::cout.rdbuf(nullptr); }
#endif
};

#define DBG (X(), std::cout << __FILE__ << " " << __LINE__ << " ")

#endif // __debug_h__
