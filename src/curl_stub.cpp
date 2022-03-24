#include <syslog.h>
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>
#include <fstream>
#include "cpp_if.hpp"
#include "common.hpp"
#include "curl_stub.h"

using namespace std;


struct write_adapter {
	const char *readptr;
	size_t sizeleft;
};


/* called us to fill the POST data */
/* the libcurl pass us userp, which is given in CURLOPT_READDATA */
static size_t read_callback(char *dest, size_t size, size_t nmemb, void *userp)
{
	struct write_adapter *wt = (struct write_adapter *)userp;
	size_t buffer_size = size*nmemb;

	if(wt->sizeleft) {
		/* copy as much as possible from the source to the destination */
		size_t copy_this_much = wt->sizeleft;
		if(copy_this_much > buffer_size)
			copy_this_much = buffer_size;
		memcpy(dest, wt->readptr, copy_this_much);

		wt->readptr += copy_this_much;
		wt->sizeleft -= copy_this_much;
		return copy_this_much; /* we copied this many bytes */
	}

	return 0; /* no more data left to deliver */
}


size_t write_recved(void *ptr, size_t size, size_t nmemb, std::string* data) {
	data->append((char*) ptr, size * nmemb);
	return size * nmemb;
}

vector<string>
get_cookies(CURL *curl)
{
	CURLcode res;
	struct curl_slist *cookies;
	struct curl_slist *nc;
	int i;
	vector<string> cookie_vec;

	setlogmask (LOG_UPTO (LOG_NOTICE));
        openlog("curl_stub", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if(res != CURLE_OK) {
		syslog( LOG_INFO, "Curl curl_easy_getinfo failed: %s", curl_easy_strerror(res));
		exit(1);
	}
	nc = cookies;

	i = 1;
	while (nc) {
		cookie_vec.push_back( string(nc->data) );
		syslog( LOG_INFO, "Cookies[%d]:%s", i, nc->data);
		nc = nc->next;
		i++;
	}
	if (i == 1) {
		syslog(LOG_INFO, "no cookies found, cannot continue");
	}
	curl_slist_free_all(cookies);
	closelog();
	return cookie_vec;
}


string find_csrf_token_in_cookie(string cookie_string)
{
	string result;
	regex reg("csrftoken\\s+(.*)");
	smatch sm;
	setlogmask (LOG_UPTO (LOG_NOTICE));
        openlog("curl_stub", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	if (regex_search(cookie_string, sm, reg) ) {
		syslog( LOG_INFO, "found CSRF token in cookie: %s", sm[1].str().c_str() );
		result = sm[1].str();
	} else {
		syslog(LOG_INFO, "no CSRF token found" );
	}
	closelog();
	return result;
}

// 2-stages, get CSRF token first, then use CSRF token to do POST
// return 0 if post succeed, otherwise return -1

long curl_post(string target_ip, int port, string api, string post_data)
{
        setlogmask (LOG_UPTO (LOG_NOTICE));
        openlog("curl_stub", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

        string url_api = "http://" + target_ip + ":" + std::to_string(port) + "/ulog/getToken";
	syslog( LOG_INFO, "url = %s", url_api.c_str());

        struct write_adapter wt;
	auto curl = curl_easy_init();
	long response_code;

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url_api.c_str());

		std::string response_string;
		std::string header_string;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_recved);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
		/* save cookies in memory */
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

		curl_easy_perform(curl);
		syslog( LOG_INFO, "header_string: %s", header_string.c_str());
		syslog( LOG_INFO, "response_string: %s",  response_string.c_str());
		vector<string> cookie_vec  = get_cookies(curl);
		if (!cookie_vec.empty()) {
			syslog( LOG_INFO, "cookie_string: %s" , cookie_vec[0].c_str());
		} else {
			return -1;
		}
		// regex search CSRF token
		string csrftok = find_csrf_token_in_cookie(cookie_vec[0]);

		// prepare post data
		wt.readptr = post_data.data();
		wt.sizeleft = post_data.size();

		// prepare extra header
		url_api = "http://" + target_ip + ":" + std::to_string(port) + api;
			cout << "url = " << url_api;

                struct curl_slist *chunk = NULL;
		string csrf_head = "X-CSRFToken: " + csrftok;
		chunk = curl_slist_append(chunk, csrf_head.c_str());

		/* Header has CSRF token */
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

		curl_easy_setopt(curl, CURLOPT_URL, url_api.c_str());

		/* Now specify we want to POST data */
		curl_easy_setopt(curl, CURLOPT_POST, 1L);

		/* we want to use our own read function */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

		/* pointer to pass to our read function */
		curl_easy_setopt(curl, CURLOPT_READDATA, &wt);

		/* get verbose debug output please */
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

                response_code = curl_easy_perform(curl);

                curl_easy_cleanup(curl);
		curl = NULL;
	}
	closelog();
	return response_code;
}
