#ifndef __HANDLE_MANAGER_HPP__
#define __HANDLE_MANAGER_HPP__

#include <algorithm>
#include <memory>
#include <mutex>
#include "cpp_if.hpp"
#include "rfid_if.hpp"

using namespace std;

class HandleUnit{
public:
	HandleUnit() {
		buffer_ptr = make_shared<string>(string{});
	}
	HANDLE handle_id;
	shared_ptr<RfidInterface> rfid_ptr;
	shared_ptr<string> buffer_ptr;
};

class HandleManager {
public:
	//mutex handle_manager_mutex;
	vector<HandleUnit> handles;
	mutex handles_mutex;

	HANDLE get_new_handle_id();
        bool is_valid_handle(HANDLE handle_id);
        size_t size() { return handles.size(); }

        vector<HandleUnit>::iterator find_handle(HANDLE handle_id);
        vector<HandleUnit>::iterator find_rfid_ptr(shared_ptr<RfidInterface> rfid_ptr);
        shared_ptr<RfidInterface> get_rfid_ptr(HANDLE handle_id);
	void append_data(HANDLE handle_id, string data);
	char* get_data(HANDLE handle_id, int* len);
        HANDLE add_handle_unit(shared_ptr<RfidInterface> rfid_ptr);
	void remove_handle_unit(HANDLE handle_id);
	void clear_buffer(HANDLE handle_id);
};


#endif // __HANDLE_MANGER_HPP__
