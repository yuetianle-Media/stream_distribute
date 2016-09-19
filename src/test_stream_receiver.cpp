#include "stdafx.h"
#include "test_stream_receiver.h"
void test_stream_receive(const std::string &url)
{
	StreamReceiver stream_receiver(url);
	stream_receiver.start();
	while (1)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}
};