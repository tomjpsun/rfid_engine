#ifndef _CMD_HANDLER_HPP_
#define _CMD_HANDLER_HPP_

#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <future>
#include <atomic>

#include "async_socket.hpp"

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
	        p_buffer_t parse(p_buffer_t p_buf, int cmd_id);
	};

	class CmdHandler {
		int create_socket(string ip, int port=1001);
		void recv_callback(string in_data);
		atomic<bool> thread_exit;
		atomic<bool> thread_ready;
		std::thread receive_thread;
		string ip;
		int port;
		int my_socket;
		int loop;
		int notify_pipe[2];
	public:
		CmdHandler(string ip, int port=1001, int loop_count=1000);
		~CmdHandler();
		void send(buffer_t cmd);
		void reply_thread_func(string ip, int port);
		void set_poll_fd(struct pollfd* p_poll_fd);
	};

}
#endif // _CMD_HANDLER_HPP_
