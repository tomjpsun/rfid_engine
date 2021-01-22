#ifndef _PACKET_QUEUE_HPP_
#define _PACKET_QUEUE_HPP_

#include <deque>
#include <mutex>

using namespace std;

template <typename Container>
class PacketQueue
{
public:
	// append elements in container
	void push_back(Container container);

	// look at elements of index, default from head
	string peek(int index=1);

	// remove elements from given index, return the removed one
	string& remove(int index);

	deque<string> queue;
	mutex queue_mutex;
};

#endif //_PACKET_QUEUE_HPP_
