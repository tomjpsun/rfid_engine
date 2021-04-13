#ifndef _SEND_SYNC_OBSERVER_HPP_
#define _SEND_SYNC_OBSERVER_HPP_

#include <mutex>
#include <condition_variable>
#include "cpp_if.hpp"
#include "observer.hpp"
#include "aixlog.hpp"


using namespace std;

class SendSyncObserver : public Observer {
public:
	SendSyncObserver( const int state = 0 )
		:observer_state( state )
		{}
	~SendSyncObserver() {}
	virtual int get_state() { return observer_state; }
	virtual void update( Subject *subject )	{
		observer_state = subject->get_state();
		LOG(SEVERITY::TRACE) << ", SyncSendObserver updated: "
				     << observer_state
				     << endl;
		cond.notify_one();
	}
	void wait() {
		unique_lock<mutex> lock(sync);
		cond.wait(lock);
	}
private:
	int observer_state;
	std::mutex sync;
	condition_variable cond;
};

class SendAsyncObserver : public Observer {
public:
	SendAsyncObserver( AsyncCallackFunc callback, void* user, const int state = 0 )
		:callback( callback ), user(user), observer_state( state )
		{}
	~SendAsyncObserver() {}
	virtual int get_state() { return observer_state; }
	virtual void update( Subject *subject )	{
		observer_state = subject->get_state();
		LOG(SEVERITY::TRACE) << ", SendAsyncObserver updated: "
				     << observer_state
				     << endl;
		if (callback(user)) {
			cond.notify_one();
		}
	}
	void wait() {
		unique_lock<mutex> lock(sync);
		cond.wait(lock);
	}
private:
	AsyncCallackFunc callback;
	void* user;
	int observer_state;
	std::mutex sync;
	condition_variable cond;
};


#endif // _SEND_SYNC_OBSERVER_HPP_
