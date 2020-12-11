#include <sstream>
#include "common.hpp"

using namespace std;
std::string hex_dump(void* from, int count)
{
    uint8_t *start = (uint8_t *)from;
    ostringstream ostr;

    uint8_t table[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    if (start != nullptr) {
        for (int i=0; i<count; i++) {
            uint8_t value = *(start + i);
	    ostr << " ";
            ostr <<  table[(value >> 4)] << table[value & 0x0F];
            if ((i+1)%16 == 0)
		    ostr << endl;
        }
    }
    return ostr.str();
}

template<typename T>
void concat( vector<T>& dest, vector<T>& src )
{
	dest.insert(
		dest.end(),
		std::make_move_iterator(src.begin()),
		std::make_move_iterator(src.end())
		);
}
