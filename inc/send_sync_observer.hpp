#ifndef _SEND_SYNC_OBSERVER_HPP_
#define _SEND_SYNC_OBSERVER_HPP_

#include <mutex>

#include "observer.hpp"

class SendSyncObserver : public Observer {
public:
	SendSyncObserver( const int state )
		:observer_state( state )
		{
			pcond_var = shared_ptr<condition_variable>(new condition_variable());

		}
	~SendSyncObserver() {}
	int get_state() { return observer_state; }
	virtual void update( Subject *subject )	{
		observer_state = subject->get_state();
		LOG(SEVERITY::TRACE) << ", SyncSendObserver updated: "
				     << observer_state
				     << endl;
		pcond_var->notify_one();
	}
	void wait() {
		unique_lock<mutex> lock(sync);
		pcond_var->wait(lock);
	}
private:
	int observer_state;
	std::mutex sync;
	std::shared_ptr<condition_variable> pcond_var;
};


#endif // _SEND_SYNC_OBSERVER_HPP_
