#pragma once
#include "stream_receiver.h"
#include "stream_sender.h"
inline void test_stream_sender(const string &url, const string &multi_server, const int &port)
{
	StreamReceiver stream_receiver(url);
	StreamSender stream_sender;
	stream_sender.add_sender_address(multi_server, port);
	stream_receiver.subcribe_ts_callback(boost::bind(&StreamSender::stream_receive_callback, &stream_sender, _1,_2));
	stream_receiver.start();
	stream_sender.start();
	while (1)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}

};
