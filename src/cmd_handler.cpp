#include "cmd_handler.hpp"

using namespace rfid;


CmdHandler::CmdHandler(string ip_addr, int port_n)
{
	thread_exit.store(false);
	std::promise<int> thread_ready;
	std::future<int> future = thread_ready.get_future();
	ip = ip_addr;
	port = port_n;
	receive_thread = std::thread(&CmdHandler::reply_thread_func,
				     this,
				     ip,
				     port,
				     &thread_ready);
	future.wait();
}

void CmdHandler::reply_thread_func(string ip, int port, promise<int>* thread_ready)
{

}
