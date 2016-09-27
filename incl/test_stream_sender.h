#pragma once
#include "stream_receiver.h"
#include "stream_sender.h"
inline void test_stream_sender(const string &url, const string &multi_server, const int &port)
{
	StreamReceiver stream_receiver(url);
	stream_receiver.start();
	StreamSender stream_sender("224.1.1.1", 65000);
	stream_receiver.subcribe_ts_callback(boost::bind(&StreamSender::stream_receive_callback, &stream_sender, _1,_2));
	while (1)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}

};
