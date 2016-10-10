#include "stdafx.h"
#include "test_stream_receiver.h"
#include <iostream>
inline void receive_ts_data(char *data, const int&data_len)
{
	fstream out_file;
	out_file.open("out.ts", ios::out | ios::binary | ios::app);
	out_file.write(data, data_len);
	out_file.close();
};
void test_stream_receive(const std::string &url)
{
	StreamReceiver stream_receiver(url);
	stream_receiver.start();
	stream_receiver.subcribe_ts_callback(boost::bind(receive_ts_data,_1,_2));
	while (1)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}
};

void test_stream_ts_callback(const std::string &ts_file_name)
{
	StreamReceiver stream_receiver("");
	fstream in_stream;
	boost::filesystem::path ts_path(ts_file_name);
	if (boost::filesystem::exists(ts_path))
	{
		in_stream.open(ts_file_name.c_str(), ios::in|ios::binary);
		char ts_data[32768] = { 0 };
		if (in_stream.read(ts_data, 32768))
		{
			stream_receiver.tsCallback(ts_data, 32768);
		}
	}
}