#ifndef __curl_stub_h__
#define __curl_stub_h__

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <deque>
//#include "asio.hpp"
#include "nlohmann/json.hpp"
#include "ulog_type.h"

using namespace std;
using json = nlohmann::json;
using namespace ulog_namespace;

template <typename Element>
class CurlStub
{
public:

        CurlStub( string target_ip = "192.168.88.105",
		  int port = 8000,
		  string api = "/cruise/add",
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

        void background_thread_func() {
		while(!thread_exit) {
			lock_guard<mutex> lock(queue_lock);
			int nsize =post_data_queue.size();
			if ( nsize > threshold ) {
				json j = post_data_queue;
				cout << __func__
				     << "() post data size = " << nsize
				     << ", json = " << j.dump(4) << endl;
				post_data_queue.clear();
			}
		}
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
