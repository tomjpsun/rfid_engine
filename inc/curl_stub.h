#ifndef __curl_stub_h__
#define __curl_stub_h__

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <deque>
#include <functional>
//#include "asio.hpp"
#include "nlohmann/json.hpp"
#include "ulog_type.h"

using namespace std;
using json = nlohmann::json;
using namespace ulog_namespace;

// C API to post message with CSRF token
long curl_post(string target_ip, int port, string api, string post_data);


template <typename Element>
class CurlStub
{
public:

        CurlStub( string target_ip = "192.168.88.105",
		  int port = 8000,
		  string api = "/ulog/add",
		  int ms_period = 1000,
		  int threshold = 10)

		: target_ip(target_ip),
		  port(port),
		  api(api),
		  ms_period(ms_period),
		  threshold(threshold)
		{
			thread_exit = false;
			background_thread = std::thread(&CurlStub::background_thread_func, this);
		}

        ~CurlStub() {
		thread_exit = true;
		background_thread.join();
	}

	void flush_queue(int thresh) {
		lock_guard<mutex> lock(queue_lock);
		int nsize = post_data_queue.size();
		if (nsize > thresh) {
			json j = post_data_queue;
			curl_post(target_ip, port, api, j.dump());
			post_data_queue.clear();
		}
	}

        void background_thread_func() {
		while(!thread_exit) {
			flush_queue(threshold);
		}
		flush_queue(0);
	}

        /* send message , sync controls whether to do blocking send */
	int post(Element post_data) {
		lock_guard<mutex> lock(queue_lock);
		post_data_queue.push_back(post_data);
		return 0;
	}


protected:
	string target_ip;
	int port;
	string api;
	int ms_period;
        int threshold;
	mutex queue_lock;
	atomic<bool> thread_exit;
	std::thread background_thread;
	deque<Element> post_data_queue;
};

#endif//__curl_stub_h__
