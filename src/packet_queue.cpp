
#include "packet_queue.hpp"

using namespace std;

template<typename Container>
void PacketQueue<Container>
::push_back(Container container)
{
}

// look at elements of index, default from head
template<typename Container>
string PacketQueue<Container>
::peek(int index)
{
	string pkt;
	return pkt;
}

// remove elements from given index, return the removed one
template<typename Container>
string& PacketQueue<Container>
::remove(int index)
{
	return queue[0];
}
