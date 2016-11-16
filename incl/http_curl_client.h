#pragma once

#include "pre_std_basic.h"
#include "pre_boost_basic.h"
#include "pre_regex.h"
#include "curl/curl.h"

typedef boost::signals2::signal<void(char* data, const long int &data_len, const bool &is_finished)> DATA_SIGNAL;
static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata);
static size_t content_callback(char *buffer, size_t size, size_t nitems, void *userdata);
static int tcp_option_callback(void *client_tp, curl_socket_t curlfd, curlsocktype purpose);

struct linker
{
	u_short l_onoff;
	u_short l_linker;
	linker()
		:l_onoff(1), l_linker(2)
	{
	}
};
class HttpCurlClient
{
public:
	HttpCurlClient();
	~HttpCurlClient();

public:
    static bool init();
    static void uninit();
    static std::once_flag is_init_;
	
	int get(const std::string &url, const int &timeout_ms = 5000/*unit:millsecond*/);
	boost::signals2::connection subscribe_data(const DATA_SIGNAL::slot_type &slot);
	bool unsubscribe_data();

	int _set_base_option();
	int _parse_http_header(char* data, const long int &data_len);
	int _receive_data(char *data, const long int &data_len);
private:
	size_t write_callback_func(char*data, size_t size, size_t nmemb, void *obj);
	bool _find_http_header_start(char* &dest, char*src, const int src_length);
	bool _find_http_header_end(char* &dest, char*src, const int src_length);
	CURL* session_;
	std::string url_;
	std::atomic<bool> is_data_;
	std::atomic<long int> http_content_len_;
	std::string http_header_;
	DATA_SIGNAL data_signal_;
	std::once_flag option_flag_;
};
typedef std::shared_ptr<HttpCurlClient>  HTTP_CURL_CLIENT_PTR;

