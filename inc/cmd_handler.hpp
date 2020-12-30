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

	enum { BUF_SIZE = 40 };

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
		atomic<bool> thread_exit;
		atomic<bool> thread_ready;
		std::thread receive_thread;
		string ip;
		int port;
	        p_buffer_t p_buffer;
		mutex buf_mutex;
		int my_socket;
		vector <Callback_t> cb_vec;
		mutex cb_vec_mutex;
		CmdParser parser;
	public:
		CmdHandler(string ip, int port=1001);
		~CmdHandler();
		void send_cmd(buffer_t cmd);
		void reply_thread_func(string ip, int port);
		Status send_cmd(string cmd, int timeout_ms, string& response);
		Status post_cmd(string cmd, int timeout_ms);
		void register_callback(Callback_t cb);
		void unregister_callback(Callback_t cb);
	};
}
#endif // _CMD_HANDLER_HPP_
