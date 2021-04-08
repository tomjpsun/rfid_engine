#ifndef __EVENT_CB_HPP__
#define __EVENT_CB_HPP__

#include <functional>
#include <string>

using event_cb = std::function<void(int cb_handle, std::string data, void *user)>;

class EventCBClass
{
public:
	EventCBClass(int cb_handle, event_cb fn, void *user = nullptr)
		: cb_handle{cb_handle}, fn{fn}, user { user }
		{}

        int cb_handle;
	event_cb fn;
	void* user;

};


#endif
