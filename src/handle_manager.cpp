#include "handle_manager.hpp"

HANDLE HandleManager::get_new_handle_id() {
	static HANDLE flow_count = 0;
	return flow_count++;
}

bool HandleManager::is_valid_handle(HANDLE handle_id) {
	auto iter = find_handle( handle_id );
	return ( iter != handles.end() );
}


vector<HandleUnit>::iterator
HandleManager::find_handle(HANDLE handle_id) {
	vector<HandleUnit>::iterator iter =
		find_if( handles.begin(), handles.end(),
			 [handle_id](HandleUnit unit){
				 return( unit.handle_id == handle_id ); });
	return iter;
}

vector<HandleUnit>::iterator
HandleManager::find_rfid_ptr(shared_ptr<RfidInterface> rfid_ptr) {
	vector<HandleUnit>::iterator iter =
		find_if( handles.begin(), handles.end(),
			 [rfid_ptr](HandleUnit unit){
				 return( unit.rfid_ptr.get() == rfid_ptr.get() ); });
		return iter;
}

shared_ptr<RfidInterface>
HandleManager::get_rfid_ptr(HANDLE handle_id) {
	auto iter = find_handle( handle_id );
	if ( iter != handles.end() ) {
		return iter->rfid_ptr;
	} else {
		return nullptr;
	}
}

void HandleManager::append_data(HANDLE handle_id, string data) {
	std::lock_guard<std::mutex> guard(handles_mutex);
	auto iter = find_handle( handle_id );
	if ( iter != handles.end() ) {
		*(iter->buffer_ptr) = *(iter->buffer_ptr) + data;
	} else {
		LOG(SEVERITY::ERROR) << LOG_TAG << "invalid handle_id: " << handle_id << endl;
	}
}

char* HandleManager::get_data(HANDLE handle_id, int* len) {
	//std::lock_guard<std::mutex> guard(handles_mutex);
	auto iter = find_handle( handle_id );
	if ( iter != handles.end() ) {
		*len = iter->buffer_ptr->size();
		return (char*)(iter->buffer_ptr->data());
	} else {
		LOG(SEVERITY::ERROR) << LOG_TAG << "invalid handle_id: " << handle_id << endl;
		*len = 0;
		return nullptr;
	}
}

HANDLE HandleManager::add_handle_unit(shared_ptr<RfidInterface> rfid_ptr) {
	std::lock_guard<std::mutex> guard(handles_mutex);
	auto iter = find_rfid_ptr( rfid_ptr );

	HANDLE new_handle_id = RFID_ERR_INVALID_HANDLE;
	if ( iter != handles.end() ) // we already have it
		return iter->handle_id;
	else {
		new_handle_id = get_new_handle_id();
		HandleUnit unit;
                unit.handle_id = new_handle_id;
		unit.rfid_ptr = rfid_ptr;

		handles.push_back(unit);
		LOG(SEVERITY::TRACE) << LOG_TAG << "add new HandleUnit( handle_id = "
				     << new_handle_id << " )" << endl;
		return new_handle_id;
	}
}

void HandleManager::remove_handle_unit(HANDLE handle_id) {
	std::lock_guard<std::mutex> guard(handles_mutex);
	auto iter = find_handle( handle_id );
	if ( iter != handles.end() ) {
		handles.erase(iter);
	} else {
		LOG(SEVERITY::WARNING) << LOG_TAG << "remove invalid handle_id: " << handle_id << endl;
	}
}

void HandleManager::clear_buffer(HANDLE handle_id) {
	//std::lock_guard<std::mutex> guard(handles_mutex);
	auto iter = find_handle(handle_id);
	if ( iter != handles.end() ) {
		iter->buffer_ptr = make_shared<string>(string{});
	} else {
		LOG(SEVERITY::WARNING) << LOG_TAG
				       << "remove invalid handle_id: " << handle_id << endl;
	}
}
