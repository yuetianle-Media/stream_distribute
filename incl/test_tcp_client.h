#pragma once
#include "tcpclient.h"

inline void data_callback(char* data, const long &data_len)
{
	std::cout << "data:" << data << std::endl;
}
void test_tcp_client()
{
	int time_out_ms = 5000;
	std::shared_ptr<TCPClient>tcp_client(new TCPClient("10.160.34.115", 91, time_out_ms));
	if (E_OK != tcp_client->connect())
	{
		return;
	}
	bool is_receive = false;
	do
	{

		std::stringstream ss;
		ss << "GET " << "/" << "119/119.m3u8" << " HTTP/1.1" << "\r\n"\
			<< "Host: " << "10.160.34.115" << ":" << 91 << " \r\n"\
			<< "User-Agent: " << "me-test" << "\r\n"\
			<< "Accept: " << "test/html,application/xhtml+xml,application/xml"\
			<< "\r\n" << "Connection: " << "keep-alive" << "\r\n\r\n";
		std::string http_m3u8_cmd = ss.str();
		tcp_client->subcribe_data_callback(boost::bind(data_callback, _1, _2));
		if (E_OK == tcp_client->async_send(http_m3u8_cmd.data(), http_m3u8_cmd.length()))
		{
			if (!is_receive)
			{
				is_receive = true;
			}
		}
		else
		{
			tcp_client->connect();
		}
	} while (!is_receive);

	std::this_thread::sleep_for(std::chrono::seconds(15));
	tcp_client->wait_response();
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}
