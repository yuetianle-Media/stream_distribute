#include "stdafx.h"
#include "test_http_client.h"
#include "pre_boost_basic.h"
#include "pre_std_basic.h"
using namespace boost::network;
#include <array>
struct m3u8_content_example
{
	char data[1024];
	m3u8_content_example()
	{
		memset(data, 0, sizeof(data));
	};
};

struct ts_data_example
{
	explicit ts_data_example(std::string &m3u8_data) :ts_data(m3u8_data)
	{
	};
	boost::system::error_code ec;
	BOOST_NETWORK_HTTP_BODY_CALLBACK(operator(), ts_iter, error)
	{
		if (!error)
		{
			ts_data.append(std::begin(ts_iter), std::end(ts_iter));
		}
	}
	std::string &ts_data;
};
void test_http_client()
{
	http::client http_client_;
	http::client::request request("http://10.160.34.115:91/119/119.m3u8");

	for (int i = 0; i < 4; i++)
	{
		std::thread task([&]()
		{
			int task_num = i;
			while (1)
			{
				std::string out_data;
				ts_data_example m3u8_example(out_data);
				http::client::response response = http_client_.get(request, m3u8_example);
				std::cout << "out_data:" << out_data << std::endl;
				status(response);
				//std::cout << body(response) << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});
		task.detach();
	}
	while (true)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}
	//while (1)
	//{
	//	std::string cur_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
	//	//http::client::response response = http_client_.get(request);
	//	//std::cout << body(response) << std::endl;
	//	//std::this_thread::sleep_for(std::chrono::seconds(5));
	//}
}

