#include <curl/curl.h>
#include <string>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>
#include <fstream>
#include "curl_stub.h"
#include "debug.h"

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

	DBG << "Cookies, curl knows:";
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if(res != CURLE_OK) {
		DBG << "Curl curl_easy_getinfo failed: "
		    << curl_easy_strerror(res);
		exit(1);
	}
	nc = cookies;

	i = 1;
	while (nc) {
		cookie_vec.push_back( string(nc->data) );
		DBG << "[" <<  i << "] " << nc->data;
		nc = nc->next;
		i++;
	}
	if (i == 1) {
		DBG << "no cookies found, cannot continue";
	}
	curl_slist_free_all(cookies);
	return cookie_vec;
}


string find_csrf_token_in_cookie(string cookie_string)
{
	string result;
	regex reg("csrftoken\\s+(.*)");
	smatch sm;
	if (regex_search(cookie_string, sm, reg) ) {
		DBG << "found CSRF token in cookie: " << sm[1].str() << endl;
		result = sm[1].str();
	} else {
		DBG << "no CSRF token found" << endl;
	}
	return result;
}

// 2-stages, get CSRF token first, then use CSRF token to do POST
// return 0 if post succeed, otherwise return -1

long curl_post(string target_ip, int port, string api, string post_data)
{
	string url_api = "http://" + target_ip + ":" + std::to_string(port) + "/ulog/getToken";
	DBG << "url = " << url_api;
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
		DBG << "header_string: " << header_string;
		DBG << "response_string: " << response_string;
		vector<string> cookie_vec  = get_cookies(curl);
		if (!cookie_vec.empty()) {
			DBG << "cookie_string: " << cookie_vec[0] << endl;
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
		DBG << "url = " << url_api;

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
	return response_code;
}

/*
int golden_test()
{
        string site = "http://192.168.88.105";
        int port = 8000;
        std::string project_dir = std::string(PROJECT_DIR);
        std::string post_file = project_dir + "/tag.json";


        // prepare post_fs: post file stream
        std::ifstream post_fs(post_file);
        if (post_fs) {
                std::string post_data((std::istreambuf_iterator<char>(post_fs)),
std::istreambuf_iterator<char>()); int res = curl_post(site, port, post_data);
                if (CURLE_OK == res) {
                        return 0;
                } else {
                        DBG << "CURL Error code : " << res << endl;
                        return -1;
                }

        } else  {
                DBG << "file " << post_file << " missing" << endl;
                return -1;
        }
}

int main(int argc, char **argv)
{
        golden_test();
        return 0;
}
*/
