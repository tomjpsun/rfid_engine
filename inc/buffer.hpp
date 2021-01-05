#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

#include <vector>
#include <mutex>
#include <atomic>
#include <utility>
#include <initializer_list>
#include "aixlog.hpp"
#include "asio/asio.hpp"

// Buffer of type <T> with
template <typename T>

class Buffer
{
public:
	Buffer(std::vector<T> vec): internal_buffer(vec) {}
	Buffer(std::initializer_list<T> vec): internal_buffer(vec) {}
	// copy constructor
	Buffer( Buffer& other ) {
		LOG(TRACE) << " copy constructor" << std::endl;
		internal_buffer = other.cp_range(0, other.size());
	}

	~Buffer() {}

	T& operator [](int index) {
		if ((size_t)index >= internal_buffer.size()) {
			LOG(DEBUG) << "[" << internal_buffer.size() << "] illegal index: " << index << std::endl;
			return *internal_buffer.end();
		}
		std::lock_guard<std::mutex> lck(mutex);
		return internal_buffer[index];
	}
	// concat 'other' to me, use move operation to speed-up
	void emplace_back(std::vector<T> other) {
		if ( !other.empty() ) {
			std::lock_guard<std::mutex> lck(mutex);
			internal_buffer.insert(	internal_buffer.end(),
						std::make_move_iterator(other.begin()),
						std::make_move_iterator(other.end()) );
		}
	}

	void emplace_back(T* start, int count) {
		if ( count > 0 ) {
			std::lock_guard<std::mutex> lck(mutex);
			internal_buffer.insert(	internal_buffer.end(),
						std::make_move_iterator(start),
						std::make_move_iterator(start + count) );
		}
	}

	void inline emplace_back(Buffer& other) {
		return emplace_back(other.cp_range(0, other.size()));
	}
	void reset() {
		std::lock_guard<std::mutex> lck(mutex);
		internal_buffer.reset();
	}

	typename std::vector<T>::size_type inline size() {
		std::lock_guard<std::mutex> lck(mutex);
		return internal_buffer.size();
	}

	bool inline empty() {
		return (size() == 0);
	}

	// remove [left, right) from internal_buffer
	std::vector<T> pop_range(int left, int right) {
		if ( (size_t)left > internal_buffer.size() ||
		     (size_t)right > internal_buffer.size() ) {
			LOG(DEBUG) << "out of range error" << std::endl;
			return std::vector<T>{};
		}
		std::lock_guard<std::mutex> lck(mutex);
		typename std::vector<T>::iterator iter = internal_buffer.begin();
		std::vector<T> new_buffer( iter + left, iter + right );
		internal_buffer.erase( iter + left, iter + right );
		return new_buffer;
	}

	// copy [left, right) from internal_buffer
	std::vector<T> cp_range(int left, int right) {
		if ( (size_t)left > internal_buffer.size() ||
		     (size_t)right > internal_buffer.size() ) {
			LOG(DEBUG) << "(" << internal_buffer.size()
				   << ") out of range error: left = " << left
				   << ", right = " << right << std::endl;
			return std::vector<T>{};
		}
		std::lock_guard<std::mutex> lck(mutex);
		typename std::vector<T>::iterator iter = internal_buffer.begin();
		std::vector<T> new_buffer( iter + left, iter + right );
		return new_buffer;
	}

	T pop() {
		if (!empty()) {
			auto v = pop_range(0, 1);
			return v[0];
		} else {
			LOG(DEBUG) << "internal buffer is empty" << std::endl;
			return T{};
		}
	}

	// find position of value in internal buffer, otherwise return -1 //
	int find(const T& value, int start_index = 0) {
		int pos = -1;
		std::lock_guard<std::mutex> lck(mutex);
		for (size_t i=start_index; i<internal_buffer.size(); i++) {
			if (internal_buffer[i] == value) {
				pos = i;
				break;
			}
		}
		return pos;
	}

	std::pair<int, int> find(std::pair<T, T> range) {
		int left_pos = find(range.first);
		int right_pos = find(range.second, left_pos);
		return std::pair<int, int>(left_pos, right_pos);
	}
private:
	std::mutex mutex;
	std::vector<T> internal_buffer;
};

#endif
