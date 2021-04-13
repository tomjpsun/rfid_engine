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


        void async_send( const std::vector<uint8_t>& cmd, AsyncCallackFunc callback, void* user=nullptr ) {
		LOG(SEVERITY::DEBUG) << "enter async_send" << endl;
		async_obs = shared_ptr<SendAsyncObserver>
			(new SendAsyncObserver(callback, user));
                cmd_handler.get_packet_queue()->attach(async_obs);
		cmd_handler.send(cmd);

		// wait until callback returns true
                async_obs->wait();
		LOG(SEVERITY::DEBUG) << "leave async_send wait" << endl;
		cmd_handler.get_packet_queue()->detach(async_obs);

	}

	void async_send(const void* pbuf, int length, AsyncCallackFunc callback, void* user=nullptr) {
		uint8_t *p = (uint8_t*) pbuf;
		std::vector<uint8_t> vbuf{ p, p+length };
		async_send(vbuf, callback, user);
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


private:
	std::shared_ptr<PacketQueue<PacketUnit>> get_packet_queue() {
		return cmd_handler.get_packet_queue();
	}

        CmdHandler cmd_handler;
	PQParams pq_params;

        mutex event_cb_map_lock;
	std::shared_ptr<SendSyncObserver> obs;
	std::shared_ptr<SendAsyncObserver> async_obs;
};

#endif // _CONNECTION_HPP_
