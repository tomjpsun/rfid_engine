#ifndef _SEND_SYNC_OBSERVER_HPP_
#define _SEND_SYNC_OBSERVER_HPP_

#include <mutex>
#include <condition_variable>
#include "cpp_if.hpp"
#include "observer.hpp"
#include "aixlog.hpp"


using namespace std;

class SendSyncObserver : public Observer<PacketContent> {
public:
	SendSyncObserver( const PacketContent state )
		:observer_state( state )
		{}
	~SendSyncObserver() {}
	virtual PacketContent get_state() { return observer_state; }
	virtual void update( Subject<PacketContent> *subject )	{
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
	PacketContent observer_state;
	std::mutex sync;
	condition_variable cond;
};

class SendAsyncObserver : public Observer<PacketContent> {
public:
	SendAsyncObserver( AsyncCallackFunc callback, void* user, const PacketContent state )
		:callback( callback ), user(user), observer_state( state )
		{}
	~SendAsyncObserver() {}
	virtual PacketContent get_state() { return observer_state; }
	virtual void update( Subject<PacketContent> *subject )	{
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
	PacketContent observer_state;
	std::mutex sync;
	condition_variable cond;
};


#endif // _SEND_SYNC_OBSERVER_HPP_
