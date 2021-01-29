#ifndef _PACKET_QUEUE_HPP_
#define _PACKET_QUEUE_HPP_

#include <deque>
#include <mutex>

using namespace std;

template <typename PacketUnit>
class PacketQueue
{
public:
	// append elements in container
	void push_back(const PacketUnit& packet) {
		lock_guard<mutex> guard(queue_mutex);
		queue.push_back(packet);
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

private:
	deque<PacketUnit> queue;
	mutex queue_mutex;
};

#endif //_PACKET_QUEUE_HPP_
