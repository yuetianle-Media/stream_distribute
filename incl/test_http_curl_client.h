#pragma once
#include "http_curl_client.h"
using namespace std;
//#include <functional>

void m3u8_callback(char *data, const long int &data_len, const bool &is_finished)
{
	std::cout << "tslen:" << data_len << "data:" << data << std::endl;
}
class m3u8_obj
{
public:
	void m3u8_callback(char *data, const long int &data_len, const bool &is_finished)
	{
		std::cout << "tslen:" << data_len << "data:" << data << std::endl;
	}
};
void test_http_curl_client()
{
	HttpCurlClient::init();
	HttpCurlClient client;
	//client.subscribe_data(boost::bind(m3u8_callback, _1, _2));
	m3u8_obj m3;
	client.subscribe_data(boost::bind(&m3u8_obj::m3u8_callback, m3, _1, _2, _3));
	client.get("http://10.160.34.115:91/119/119.m3u8");
	std::this_thread::sleep_for(std::chrono::seconds(60*3));
	HttpCurlClient::uninit();
}
