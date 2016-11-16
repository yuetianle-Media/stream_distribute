#include "stdafx.h"
#include "test_http_client.h"
using namespace boost::network;
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
				http::client::response response = http_client_.get(request);
				if (200 != response.status())
				{
				}
				vvlog_i("task:" <<task_num);
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

