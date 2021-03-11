#ifndef _PACKET_QUEUE_HPP_
#define _PACKET_QUEUE_HPP_

#include <deque>
#include <mutex>
#include "observer.hpp"

using namespace std;

template <typename PacketUnit>
class PacketQueue: public Subject
{
public:
	PacketQueue() {}

	virtual ~PacketQueue() {}

	virtual int get_state() {
		return subject_state;
	}

	virtual void set_state( const int s ) {
		subject_state = s;
	}

	// Since mutex_lock is 'non-copyable' (or cause problem if it is 'locking' on copy)
	// So class contains mutex_lock should copy with care --- don't use default copy ctor',
	// each PacketQueue's lock protect its own queue, so we only copy the embedded
	// queue but the lock

	PacketQueue(PacketQueue<PacketUnit>& other) {
		for (int i=0; i<other.size(); i++)
			push_back(other.peek(i));
	}

	// append elements in container
	void push_back(const PacketUnit& packet) {
		lock_guard<mutex> guard(queue_mutex);
		queue.push_back(packet);
		set_state(packet.packet_type);
		notify();
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
		try {
			PacketUnit p = queue.at(index);
			queue.erase(queue.begin() + index);
			return p;
		}
		catch (std::out_of_range const& e){
			std::cout << e.what() << std::endl;
		}
		return PacketUnit();
	}

	ssize_t size() { return queue.size(); }
	void reset() {
		lock_guard<mutex> guard(queue_mutex);
		queue = deque<PacketUnit>{};
	}

private:
	mutex queue_mutex;
	deque<PacketUnit> queue;
	int subject_state;
};

#endif //_PACKET_QUEUE_HPP_
