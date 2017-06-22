#include "stdafx.h"
#include "http_curl_client.h"

std::once_flag HttpCurlClient::is_init_;
HttpCurlClient::HttpCurlClient()
	:is_data_(false)
{
	session_ = curl_easy_init();
}
HttpCurlClient::~HttpCurlClient()
{
	if (session_)
	{
		curl_easy_cleanup(session_);
	}
	std::cout << "come destruct http client!!" << std::endl;
}
bool HttpCurlClient::init()
{
	CURLcode result = CURLE_OK;
	std::call_once(is_init_, [&result]()
	{
		result = curl_global_init(CURL_GLOBAL_ALL);
	});
	if (CURLE_OK != result)
	{
		std::cout << "init faile, error:" << result << std::endl;
		return false;
	}
	return true;
}

void HttpCurlClient::uninit()
{
	std::call_once(is_init_, []() 
	{
		curl_global_cleanup();
	});
}

int HttpCurlClient::get(const std::string &url, const int &timeout_ms /*= 5000/*unit:millsecond*/)
{
	CURLcode result = CURLE_OK;
	std::call_once(option_flag_, [&]() {
		_set_base_option();
	});
	url_ = url;
	if (session_)
	{
		//result = curl_easy_setopt(session_, CURLOPT_SOCKOPTFUNCTION, tcp_option_callback);
		//curl_easy_setopt(session_, CURLOPT_SOCKOPTDATA, this);
		result = curl_easy_setopt(session_, CURLOPT_URL, (char*)url.data());
		if (CURLE_OK != result)
		{
			return result;
		}
		//curl_easy_setopt(session_, CURLOPT_VERBOSE, 1L);
		//curl_easy_setopt(session_, CURLOPT_NOSIGNAL, 1L);

		//curl_easy_setopt(session_, CURLOPT_FOLLOWLOCATION, 1L);//支持重定向
		//curl_easy_setopt(session_, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);
		//curl_easy_setopt(session_, CURLOPT_HEADERFUNCTION, header_callback);
		/*curl_easy_setopt(session_, CURLOPT_HEADERFUNCTION\
			, [](char *buffer, size_t size, size_t nitems, void *userdata)->size_t
		{
			std::cout << "header comehere" << std::endl;
			//if (userdata)
			//{
			//	http_curl_client *obj = (http_curl_client*)userdata;
			//	obj->_parse_http_header(buffer, size*nitems);
			//}
		});
		*/
		//curl_easy_setopt(session_, CURLOPT_HEADERDATA, this);
		//curl_easy_setopt(session_, CURLOPT_WRITEFUNCTION, content_callback);
		/*curl_easy_setopt(session_, CURLOPT_WRITEFUNCTION, [](char *buffer, size_t size, size_t nitems, void *userdata)->size_t 
		{
			std::cout << "ts data come here" << std::endl;
			//if (userdata)
			//{
			//	http_curl_client *obj = (http_curl_client*)userdata;
			//	obj->_parse_http_header(buffer, size*nitems);
			//}
		});
		*/
		//curl_easy_setopt(session_, CURLOPT_WRITEDATA, this);
		result = curl_easy_perform(session_);
		if (result == CURLE_OK)
		{
			long response_code = 0;
			double total_time = 0;
			curl_easy_getinfo(session_, CURLINFO_RESPONSE_CODE, &response_code);
			curl_easy_getinfo(session_, CURLINFO_TOTAL_TIME, &total_time);
			if (response_code != 200)
			{
				return response_code;
			}
			else
			{
				//std::cout << "url:" << url << "time:" << total_time << std::endl;
			}
		}
		is_data_ = false;
		return result;
	}
	else
	{
		return -1;
	}
}

boost::signals2::connection HttpCurlClient::subscribe_data(const DATA_SIGNAL::slot_type &slot)
{
	return data_signal_.connect(slot);
}

bool HttpCurlClient::unsubscribe_data()
{
	if (!data_signal_.empty())
	{
		data_signal_.disconnect_all_slots();
	}
	return true;
}

int HttpCurlClient::_set_base_option()
{
	CURLcode result = CURLE_OK;
	if (session_)
	{
		result = curl_easy_setopt(session_, CURLOPT_SOCKOPTFUNCTION, tcp_option_callback);
		if (CURLE_OK != result)
		{
			return result;
		}
		curl_easy_setopt(session_, CURLOPT_SOCKOPTDATA, this);
		if (CURLE_OK != result)
		{
			return result;
		}
		result = curl_easy_setopt(session_, CURLOPT_NOSIGNAL, 1L);
		if (CURLE_OK != result)
		{
			return result;
		}
		result = curl_easy_setopt(session_, CURLOPT_FOLLOWLOCATION, 1L);//支持重定向
		if (CURLE_OK != result)
		{
			return result;
		}
		//result = curl_easy_setopt(session_, CURLOPT_TIMEOUT_MS, 5000);
		//if (CURLE_OK != result)
		//{
		//	return result;
		//}
		result = curl_easy_setopt(session_, CURLOPT_CONNECTTIMEOUT_MS, 5000);
		if (CURLE_OK != result)
		{
			std::cout << "error timeout" << std::endl;
			return result;
		}
		result = curl_easy_setopt(session_, CURLOPT_HEADERFUNCTION, header_callback);
		if (CURLE_OK != result)
		{
			return result;
		}
		result = curl_easy_setopt(session_, CURLOPT_HEADERDATA, this);
		if (CURLE_OK != result)
		{
			return result;
		}
		result = curl_easy_setopt(session_, CURLOPT_WRITEFUNCTION, content_callback);
		if (CURLE_OK != result)
		{
			return result;
		}
		result = curl_easy_setopt(session_, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(session_, CURLOPT_FORBID_REUSE, 0L);
	}
	return result;
}

int HttpCurlClient::_parse_http_header(char* data, const long int &data_len)
{
	http_header_.append(data, data_len);
	char *http_header_start = nullptr;
	char *http_header_end = nullptr;
	//std::cout << "http_header len:" << data_len << "data:" << http_header_ << std::endl;
	if (_find_http_header_start(http_header_start, (char*)http_header_.data(), http_header_.length())\
		&& _find_http_header_end(http_header_end, (char*)http_header_.data(), http_header_.length()))
	{
		RegexTextFinder regex_finder;
		int index = http_header_.find_first_of("\r\n");
		int http_code = 200;
		if (index != string::npos)
		{
			string first_header_content = http_header_.substr(0, index);
			if (regex_finder.find(first_header_content, "(.+?) (\\d+) (.+$)"))
			{
				http_code = atoi(regex_finder[2].c_str());
				if (http_code == 200)
				{
					int content_index = http_header_.find("Content-Length:");
					string content_value;
					if (string::npos != content_index)
					{
						int content_end_index = http_header_.find("\r\n", content_index);
						//std::cout << "content index:" << content_index << "end:" << content_end_index << std::endl;
						content_value = http_header_.substr(content_index, content_end_index - content_index);
						if (regex_finder.find(content_value, "(.+?): (\\d+)"))
						{
							http_content_len_ = atol(regex_finder[2].c_str());
						}
					}
					//std::cout << "find httpheader and ok content value:"\
						<< content_value << "contentLen:"\
						<< http_content_len_ << "data:" << http_header_ << endl;
					is_data_ = true;
				}
				else
				{
					//vvlog_e("request error url:" << url_ << " http_header:" << http_header_);
				}
			}
		}
		http_header_.clear();
	}
	return 0;
}

int HttpCurlClient::_receive_data(char *data, const long int &data_len)
{
	//std::cout << "http_body len:" << data_len << "data:" <<data  << std::endl;
	if (!data_signal_.empty() && is_data_ && 0 < http_content_len_)
	{
		http_content_len_ -= data_len;
		//std::cout << "signal num:" << data_signal_.num_slots() << std::endl;
		if (0 >= http_content_len_)
		{
			http_content_len_ = 0;
			data_signal_(data, data_len, true);
			is_data_ = false;
		}
		else
		{
			data_signal_(data, data_len, false);
		}
	}
	else
	{
		//vvlog_e("no signal or data fail, url:" << url_ << " data:" << data\
			<< "dataLen:" << data_len);
	}
	return 0;
}

size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
	//std::cout << "header len:" << size*nitems << "data" << buffer << std::endl;
	if (userdata)
	{
		HttpCurlClient *obj = (HttpCurlClient*)userdata;
		obj->_parse_http_header(buffer, size*nitems);
	}
	return size*nitems;
}

size_t content_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
	//std::cout << "ts data len:" << size*nitems << "data:" << buffer << std::endl;
	if (userdata)
	{
		HttpCurlClient *obj = (HttpCurlClient*)userdata;
		if (buffer && 0 < size*nitems)
		{
			obj->_receive_data(buffer, size*nitems);
		}
		else
		{
			vvlog_e("content callback error!!!");
		}
	}
	return size*nitems;
}

int tcp_option_callback(void *client_tp, curl_socket_t curlfd, curlsocktype purpose)
{
	if (purpose == CURLSOCKTYPE_IPCXN)
	{
//		linker sock_linker;
//		sock_linker.l_onoff = 0;
//		sock_linker.l_linker = 3;
//		int result = setsockopt(curlfd, SOL_SOCKET, SO_LINGER, (const char*)&sock_linker, sizeof(linker));
//		if (0 != result)
//		{
//#ifdef WIN32
//			std::cout << "socket set linker error:" << result << std::endl;
//#else
//			perror("socket set linker error:");
//#endif // WIN32
//		}
		int recv_size = (int)(1024 * 1024 * 1.25);
		int result = setsockopt(curlfd, SOL_SOCKET, SO_RCVBUF, (char*)&recv_size, sizeof(recv_size));
		if (0 != result)
		{
#ifdef WIN32
			std::cout << "socket set receive error:" << result << std::endl;
#else
			perror("socket set linker error:");
#endif // WIN32
		}
	}
	return CURL_SOCKOPT_OK;
}

size_t HttpCurlClient::write_callback_func(char*data, size_t size, size_t nmemb, void *obj)
{
	return 0;
}

bool HttpCurlClient::_find_http_header_start(char* &dest, char*src, const int src_length)
{
	char *tmp_src = src;
	int tmp_length = src_length;
	while (tmp_src && 4 <= tmp_length)
	{
		if ('H' == *tmp_src && 'T' == *(tmp_src + 1) && 'T' == *(tmp_src + 2) && 'P' == *(tmp_src + 3))//"HTTP"
		{
			if (dest)
			{
				dest = tmp_src;
			}
			return true;
		}
		tmp_src++;
		tmp_length--;
	}
	return false;
}

bool HttpCurlClient::_find_http_header_end(char* &dest, char*src, const int src_length)
{
	char *tmp_src = src;
	int tmp_length = src_length;
	while (tmp_src && 4 <= tmp_length)
	{
		if ('\r'== *tmp_src && '\n'== *(tmp_src + 1) && '\r'== *(tmp_src + 2) && '\n' == *(tmp_src + 3))
		{
			if (dest)
			{
				dest = tmp_src+3;
			}
			return true;
		}
		tmp_src++;
		tmp_length--;
	}
	return false;
}

