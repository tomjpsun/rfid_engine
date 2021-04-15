#ifndef _SEND_SYNC_OBSERVER_HPP_
#define _SEND_SYNC_OBSERVER_HPP_

#include <mutex>
#include <vector>
#include <condition_variable>
#include "cpp_if.hpp"
#include "observer.hpp"
#include "aixlog.hpp"


using namespace std;

class SendAsyncObserver : public Observer<PacketContent> {
public:
	SendAsyncObserver( vector<FinishConditionType> finish_conditions,
			   AsyncCallackFunc callback, void* user )
		: finish_conditions(finish_conditions),
		  callback( callback ),
		  user(user) {}

	~SendAsyncObserver() {}

	virtual PacketContent get_state() { return observer_state; }
	virtual void update( Subject<PacketContent> *subject )	{
		observer_state = subject->get_state();
                LOG(SEVERITY::TRACE) << ", SendAsyncObserver updated: "
                                     << observer_state.to_string()
				     << endl;
		bool result = false;
		if ( callback )
			result = callback(observer_state, user);
		if ( result ) {
			cond.notify_one();
		}
		for ( FinishConditionType &f: finish_conditions ) {
			result = f(observer_state);
			if ( result ) {
				cond.notify_one();
				break;
			}
		}
		//if ( is_EOP(observer_state) )
		//	cond.notify_one();

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
