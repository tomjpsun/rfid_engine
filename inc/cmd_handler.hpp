#ifndef _CMD_HANDLER_HPP_
#define _CMD_HANDLER_HPP_

#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <future>
#include <atomic>
#include <deque>
#include <regex>
#include <memory>

#include "cpp_if.hpp"
#include "packet_queue.hpp"
#include "packet_content.hpp"
#include "aixlog.hpp"
#include "asio.hpp"

using namespace std;
using namespace asio;

using p_socket_t = std::shared_ptr<asio::ip::tcp::socket>;

namespace rfid
{
	enum Status {
		SUCCESS = 0,
		ERROR = -1,
		BUSY = -2,
		TIMEOUT = -3
	};

	enum {
		READ_ENDPOINT = 0,
		WRITE_ENDPOINT = 1
	};

	enum {
		BUF_SIZE = 1024,
		TASK_VEC_MAX_SIZE = 1000
	};


	class CmdHandler {
	public:
		CmdHandler();
		~CmdHandler();
		bool start_recv_thread(string ip_addr, int port_n);
		void stop_recv_thread();
		int send(vector<unsigned char> cmd);
		// find 'rgx' from socket buffer, put to packet queue with its type
	        bool extract(const regex rgx, int ptype);
		// receive socket dataa
		void reply_thread_func(string ip, int port, asio::error_code* ec_ptr);
		bool thread_is_ready() { return (bool) thread_ready; }
		// post process of the received data:
		//   add to buffer
		//   parse packet from buffer, put to packet queue,
		//   thread design, to prevent reply_thread_func from blocking
		void task_func(string in_data);
		size_t packet_queue_size() { return ppacket_queue->size(); }

		std::shared_ptr<PacketQueue<PacketContent>> get_packet_queue() {
			if (ppacket_queue)
				return ppacket_queue;
			else {
				LOG(ERROR) << ": return null pointer" << endl;
				return nullptr;
			}
		}
		bool is_active() {
			return receive_thread.joinable();
		}
		inline void reset_heartbeat_callback() {
			heartbeat_callback_function = nullptr;
		}

		inline void set_heartbeat_callback(HeartbeatCallbackType cb) {
			heartbeat_callback_function = cb;
		}

		inline HeartbeatCallbackType get_heartbeat_callback() {
			return heartbeat_callback_function;
		}

		void async_read_callback(const asio::error_code& ec,
					 std::size_t bytes_transferred,
					 p_socket_t p_sock);

	private:
		int create_socket(string ip, int port=1001);
		void recv_callback(string& in_data);

		atomic<bool> thread_ready;
		std::thread receive_thread;
		string ip;
		int port;

		string buffer;
		mutex buffer_mutex;
		std::shared_ptr<PacketQueue<PacketContent>> ppacket_queue;
		// careful use, not thread safe
                int task_vec_index;
                vector<shared_ptr<std::thread>> task_vec;
		HeartbeatCallbackType heartbeat_callback_function;
		unsigned char receive_buffer[BUF_SIZE];
		p_socket_t asio_socket;
	};

}
#endif // _CMD_HANDLER_HPP_
