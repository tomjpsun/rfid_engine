#ifndef _SEND_SYNC_OBSERVER_HPP_
#define _SEND_SYNC_OBSERVER_HPP_

#include <mutex>
#include <vector>
#include <condition_variable>
#include "cpp_if.hpp"
#include "observer.hpp"
#include "aixlog.hpp"


using namespace std;

enum
{
	OBSERVER_MSG_WAKEUP = 1
};

class SendSyncObserver : public Observer<PacketContent> {
public:
	SendSyncObserver() {}
	~SendSyncObserver() {}

	virtual PacketContent get_state() { return observer_state; }

	virtual void update( Subject<PacketContent> *subject )	{
		observer_state = subject->get_state();
		cond.notify_one();
	}

	virtual void msg( int msg_id, void* msg_data = nullptr) {
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
	SendAsyncObserver( vector<FinishConditionType> finish_conditions,
			   AsyncCallackFunc callback, void* user )
		: finish_conditions(finish_conditions),
		  callback( callback ),
		  user(user) {}

	~SendAsyncObserver() {}

	virtual PacketContent get_state() { return observer_state; }

	virtual void msg( int msg_id, void* msg_data = nullptr) {
                LOG(SEVERITY::NOTICE) << LOG_TAG << "get msg_id = " << msg_id << ", msg_data = " << msg_data << endl;
		switch (msg_id) {
			case OBSERVER_MSG_WAKEUP:
				cond.notify_one();
				break;
			default:
				break;
		};
	}

	// update(): First, check via user's callback,
	// then, check all finishing conditions,
	// finally, pop() on each update()
	virtual void update( Subject<PacketContent> *subject )	{
		observer_state = subject->get_state();
                LOG(SEVERITY::TRACE) << LOG_TAG << "queue size: " << subject->size() << endl;
                // if result is true, leave the wait block
		bool result = false;
		if ( callback )
			result = callback( observer_state, user );
		if ( result ) {
			cond.notify_one();
		} else {
			for ( FinishConditionType &f: finish_conditions ) {
		 		result = f( observer_state );
				if ( result ) {
					cond.notify_one();
					break;
				}
			}
		}
		subject->pop();
	}

	void wait() {
		unique_lock<mutex> lock(sync);
		cond.wait(lock);
	}


private:
	vector<FinishConditionType> finish_conditions;
	AsyncCallackFunc callback;
	void* user;
	PacketContent observer_state;
	std::mutex sync;
	condition_variable cond;
};


#endif // _SEND_SYNC_OBSERVER_HPP_
