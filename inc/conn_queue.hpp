#ifndef _CONN_QUEUE_HPP_
#define _CONN_QUEUE_HPP_
#include <memory>
#include <vector>
#include <map>
#include <functional>

#include "cpp_if.hpp"
#include "cmd_handler.hpp"
#include "send_sync_observer.hpp"


using namespace std;


template <typename PacketUnit>
class ConnQueue
{
public:
	ConnQueue() {}
        ConnQueue(const PQParams& _params) {
		set_params(_params);
	}

	void set_params(const PQParams& _params) {
		this->pq_params = _params;
	}

	PQParams get_params() {
		return pq_params;
	}

	ssize_t size() {
		return get_packet_queue()->size();
	}
	PacketUnit pop() {
		PacketUnit pkt;
		if (size() > 0) {
			pkt = get_packet_queue()->pop();
		}
		else {
			LOG(SEVERITY::ERROR) << ", Packet Queue Size 0" << endl;
		}
		return pkt;
	}
	PacketUnit peek(int index) {
		PacketUnit pkt;
		if (size() > 0) {
			pkt = get_packet_queue()->peek(index);
		}
		else {
			LOG(SEVERITY::ERROR) << ", Packet Queue Size 0" << endl;
		}
		return pkt;
	}

	void reset() {
		return get_packet_queue()->reset();
	}


        int async_send(
		int uiCommandId,
		const std::vector<uint8_t>& cmd,
		vector<FinishConditionType> finish_conditions,
		AsyncCallackFunc callback,
		void* user=nullptr ) {

		LOG(SEVERITY::DEBUG) << "enter async_send" << endl;
		async_obs = shared_ptr<SendAsyncObserver>
			(new SendAsyncObserver(finish_conditions, callback, user));
		async_obs->set_observer_id(uiCommandId);

                get_packet_queue()->attach(async_obs);

		int result = 0;

		// for empty cmd, don't send anything, just wait.
		//   the HeartBeat command use this special case
		if (cmd.size() > 0)
			cmd_handler.send(cmd);

		if ( result >= 0) {
			// wait until callback returns true
			async_obs->wait();
		} else {
			LOG(SEVERITY::ERROR) << "ERROR: " << errno << endl;
		}
		LOG(SEVERITY::DEBUG) << "leave async_send wait" << endl;
		get_packet_queue()->detach(async_obs);
		return result;

	}

	int async_send(
		int uiCommandId,
		const void* pbuf,
		int length,
		vector<FinishConditionType> finish_conditions,
		AsyncCallackFunc callback,
		void* user=nullptr) {

		uint8_t *p = (uint8_t*) pbuf;
		std::vector<uint8_t> vbuf{ p, p+length };
		return async_send(uiCommandId, vbuf, finish_conditions, callback, user);
	}


	int send(const std::vector<uint8_t>& cmd) {
		sync_obs = shared_ptr<SendSyncObserver>
			(new SendSyncObserver());
                get_packet_queue()->attach(sync_obs);

		int result = cmd_handler.send(cmd);

		if ( result >= 0) {
			// wait until callback returns true
			sync_obs->wait();
		} else {
			LOG(SEVERITY::ERROR) << "ERROR: " << errno << endl;
		}

		LOG(SEVERITY::DEBUG) << "leave sync_send wait" << endl;
		get_packet_queue()->detach(sync_obs);
		return result;
	}


        int send(const void* pbuf, int length) {
		uint8_t *p = (uint8_t*) pbuf;
		std::vector<uint8_t> vbuf{ p, p+length };
		return send(vbuf);
	}

	void send_no_wait(const vector<uint8_t>& cmd) {
		cmd_handler.send(cmd);
	}

        bool start_service() {
		PPQParams p = &pq_params;
		return cmd_handler.start_recv_thread( p->ip_addr, p->port );
	}

	void stop_service() {
		cmd_handler.stop_recv_thread();
	}

	bool is_active() {
		return cmd_handler.is_active();
	}

        std::vector<shared_ptr<Observer<PacketContent>>>
	getOberservers() {
		return cmd_handler.get_packet_queue()->observers;
	}

private:
	inline std::shared_ptr<PacketQueue<PacketUnit>> get_packet_queue() {
		return cmd_handler.get_packet_queue();
	}

        CmdHandler cmd_handler;
	PQParams pq_params;

        mutex event_cb_map_lock;

	// remember a?sync object which
	// used in the last send_a?sync command
	std::shared_ptr<SendAsyncObserver> async_obs;
	std::shared_ptr<SendSyncObserver> sync_obs;
};

#endif // _CONNECTION_HPP_
