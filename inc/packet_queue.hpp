#ifndef _PACKET_QUEUE_HPP_
#define _PACKET_QUEUE_HPP_

#include <deque>
#include <mutex>
#include "observer.hpp"
#include "aixlog.hpp"
#include "packet_content.hpp"

using namespace std;

template <typename PacketUnit>
class PacketQueue: public Subject<PacketUnit>
{
public:
	PacketQueue() {}

	virtual ~PacketQueue() {}

	virtual PacketUnit get_state() {
		return subject_state;
	}

	virtual void set_state( const PacketUnit s ) {
		subject_state = s;
	}

	// append elements in container
	void push_back(const PacketUnit& packet) {
		lock_guard<mutex> guard(queue_mutex);
		queue.push_back(packet);
		set_state(packet);
		this->notify();
	}

	// look at elements of index, default from head
	PacketUnit peek(int index=0) {
		// using queue.at() to check bounds
		try {
			PacketUnit p = queue.at(index);
			return p;
		}
		catch (std::out_of_range const& e){
			std::cout << e.what() << std::endl;
		}
		return PacketUnit();
	}

	// remove elements from given index, return the removed one,
	// default from head
	PacketUnit remove(int index=0) {
		lock_guard<mutex> guard(queue_mutex);
		typename deque<PacketContent>::iterator iter = queue.begin();
	        for ( ; iter != queue.end() && index > 0; iter++, index--)
			;
		PacketUnit p = *iter;
		queue.erase(iter);
		return p;
	}

	ssize_t size() { return queue.size(); }
	void reset() {
		lock_guard<mutex> guard(queue_mutex);
		queue = deque<PacketUnit>{};
	}

private:
	mutex queue_mutex;
	deque<PacketUnit> queue;
	PacketUnit subject_state;
};

#endif //_PACKET_QUEUE_HPP_
