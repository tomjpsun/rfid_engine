#ifndef _CONN_QUEUE_HPP_
#define _CONN_QUEUE_HPP_
#include "cpp_if.hpp"
#include "cmd_handler.hpp"
#include "send_sync_observer.hpp"
#include <memory>
#include <vector>

using namespace std;



class ConnQueue
{
public:
        ConnQueue(const PQParams& _params) {
		this->pq_params = _params;
	}
	ssize_t size() {
		return get_packet_queue()->size();
	}
	PacketContent pop() {
		PacketContent pkt;
		if (size() > 0) {
			pkt = cmd_handler.get_packet_queue()->remove();
		}
		else {
			LOG(SEVERITY::ERROR) << ", Packet Queue Size 0" << endl;
		}
		return pkt;
	}
	PacketContent peek(int index) {
		PacketContent pkt;
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

	void send(std::vector<uint8_t> cmd) {
		LOG(SEVERITY::TRACE) << "enter send" << endl;
                SendSyncObserver obs(0);
		cmd_handler.get_packet_queue()->attach(&obs);
		LOG(SEVERITY::TRACE) << "before send" << endl;
		cmd_handler.send(cmd);
                obs.wait();
		LOG(SEVERITY::TRACE) << "after send" << endl;
		cmd_handler.get_packet_queue()->detach(&obs);
	}

	void start_service() {
		PPQParams p = &pq_params;
		cmd_handler.start_recv_thread( p->ip_addr, p->port, p->loop );
	}

	void stop_service() {
		cmd_handler.stop_recv_thread();
	}

private:
	std::shared_ptr<PacketQueue<PacketContent>> get_packet_queue() {
		return cmd_handler.get_packet_queue();
	}

        CmdHandler cmd_handler;
	PQParams pq_params;
};

#endif // _CONNECTION_HPP_
