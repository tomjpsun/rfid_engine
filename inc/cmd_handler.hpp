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

#include "packet_queue.hpp"
#include "packet_content.hpp"
#include "aixlog.hpp"

using namespace std;

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

	enum { BUF_SIZE = 256 };

	using CallbackFunction = std::function<void(Status s, std::string response, void* user)>;

	struct Callback_t {
		CallbackFunction f;
		void* user;
	};

	class CmdParser {
	public:
		// find command ID in buffer,
		// return result buffer substracted the command
	        vector<unsigned char> parse(vector<unsigned char> buf, int cmd_id);
	};


	class CmdHandler {
	public:
		CmdHandler();
		~CmdHandler();
		bool start_recv_thread(string ip_addr, int port_n, int loop_count);
		void stop_recv_thread();
		void send(vector<unsigned char> cmd);
		// find 'rgx' from socket buffer, put to packet queue with its type
	        bool extract(const regex rgx, int ptype);
		// receive socket dataa
		void reply_thread_func(string ip, int port);
		bool thread_is_ready() { return (bool) thread_ready; }
		// post process of the received data:
		//   add to buffer
		//   parse packet from buffer, put to packet queue,
		//   thread design, to prevent reply_thread_func from blocking
		void process_buffer_thread_func(string in_data);
		size_t packet_queue_size() { return ppacket_queue->size(); }
		void set_poll_fd(struct pollfd* p_poll_fd);
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
	private:
		int create_socket(string ip, int port=1001);
		void recv_callback(string& in_data);
		atomic<bool> thread_exit;
		atomic<bool> thread_ready;
		std::thread receive_thread;
		string ip;
		int port;
		int my_socket;
		int loop;
		int notify_pipe[2];
		string buffer;
		mutex buffer_mutex;
		std::shared_ptr<PacketQueue<PacketContent>> ppacket_queue;
		vector<shared_ptr<thread>> process_buffer_thread_func_vec;
	};

}
#endif // _CMD_HANDLER_HPP_
