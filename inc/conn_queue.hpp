#ifndef _CONN_QUEUE_HPP_
#define _CONN_QUEUE_HPP_
#include <memory>
#include <vector>
#include <map>

#include "cpp_if.hpp"
#include "cmd_handler.hpp"
#include "send_sync_observer.hpp"
#include "event_cb.hpp"

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
			pkt = cmd_handler.get_packet_queue()->remove();
		}
		else {
			LOG(SEVERITY::ERROR) << ", Packet Queue Size 0" << endl;
		}
		return pkt;
	}
	PacketUnit peek(int index) {
		PacketUnit pkt;
		if (size() > 0) {
			pkt = cmd_handler.get_packet_queue()->peek(index);
		}
		else {
			LOG(SEVERITY::ERROR) << ", Packet Queue Size 0" << endl;
		}
		return pkt;
	}

	void reset() {
		return cmd_handler.get_packet_queue()->reset();
	}

	// return true to leave async wait
	bool async_callback() {
		static int count = 2;
		LOG(SEVERITY::DEBUG) << "enter" << endl;
		if (--count == 0)
			return true;
		else
			return false;
	}

	void async_send(const std::vector<uint8_t>& cmd) {
		LOG(SEVERITY::DEBUG) << "enter send" << endl;
		async_obs = shared_ptr<SendAsyncObserver>
			(new SendAsyncObserver(std::bind(&ConnQueue::async_callback, this)));
                cmd_handler.get_packet_queue()->attach(async_obs);
		LOG(SEVERITY::DEBUG) << "before send" << endl;
		cmd_handler.send(cmd);
                async_obs->wait();
		cmd_handler.get_packet_queue()->detach(async_obs);

	}

	void async_send(const void* pbuf, int length) {
		uint8_t *p = (uint8_t*) pbuf;
		std::vector<uint8_t> vbuf{ p, p+length };
		async_send(vbuf);
	}

	void send(const std::vector<uint8_t>& cmd) {
		LOG(SEVERITY::TRACE) << "enter send" << endl;
		obs = shared_ptr<SendSyncObserver>
			(new SendSyncObserver());
		cmd_handler.get_packet_queue()->attach(obs);
		LOG(SEVERITY::TRACE) << "before send" << endl;
		cmd_handler.send(cmd);
                obs->wait();
		LOG(SEVERITY::TRACE) << "after send" << endl;
		cmd_handler.get_packet_queue()->detach(obs);
	}

	void send(const void* pbuf, int length) {
		uint8_t *p = (uint8_t*) pbuf;
		std::vector<uint8_t> vbuf{ p, p+length };
		send(vbuf);
	}

        bool start_service() {
		PPQParams p = &pq_params;
		return cmd_handler.start_recv_thread( p->ip_addr, p->port, p->loop );
	}

	void stop_service() {
		cmd_handler.stop_recv_thread();
	}

	bool is_active() {
		return cmd_handler.is_active();
	}


	void add_cb(int cb_handle, event_cb cb_func, int count , int timeout_ms, void* user) {
		EventCBClass cb_obj { cb_handle, cb_func, user };
		lock_guard<mutex> guard(event_cb_map_lock);
		auto iter = event_cb_map.find(cb_handle);
		if (iter != event_cb_map.end())
			event_cb_map[cb_handle] = cb_obj;
		else
			LOG(SEVERITY::NOTICE) << "duplicate key: " << cb_handle << endl;
	}

	void remove_cb(int cb_handle) {
		lock_guard<mutex> guard(event_cb_map_lock);
		auto iter = event_cb_map.find(cb_handle);
		if (iter != event_cb_map.end())
			event_cb_map.erase(iter);
		else
			LOG(SEVERITY::NOTICE) << "not found key: " << cb_handle << endl;
	}

private:
	std::shared_ptr<PacketQueue<PacketUnit>> get_packet_queue() {
		return cmd_handler.get_packet_queue();
	}

        CmdHandler cmd_handler;
	PQParams pq_params;
	map<int, EventCBClass> event_cb_map;
        mutex event_cb_map_lock;
	std::shared_ptr<SendSyncObserver> obs;
	std::shared_ptr<SendAsyncObserver> async_obs;
};

#endif // _CONNECTION_HPP_
