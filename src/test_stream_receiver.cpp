#include "stdafx.h"
#include "test_stream_receiver.h"
#include <iostream>


std::string out_ts_file = "";
static FILE *open_file = nullptr;
inline void write_content_to_file(const string &out_file_name, char *data, const int &data_len)
{
	if (open_file)
	{
		int write_size = fwrite(data, sizeof(char), data_len, open_file);
		if (0 > write_size)
		{
#ifdef _WIN32
		std::cout << " file write error:" << GetLastError() << std::endl;
#else
		perror("file write error");
#endif // _DEBUG
		}
		fflush(open_file);
	}
	else
	{
#ifdef _WIN32
		std::cout << out_file_name <<" file open error:" << GetLastError() << std::endl;
#else
		//vvlog_e("open file:" << out_file_name << "error:" << errno);
		perror("file open error");
#endif // _DEBUG
	}
}
inline void receive_ts_data(char *data, const int&data_len, const bool &is_finished)
{
	write_content_to_file("ts.ts", data, data_len);
	//std::cout << "ts dataLen:" << data_len << "isfinished:" << is_finished << std::endl;
	//fstream out_file;
	//out_file.open("out.ts", ios::out | ios::binary | ios::app);
	//out_file.write(data, data_len);
	//out_file.close();
};
void test_stream_receive(const std::string &url)
{
	StreamReceiver stream_receiver(url);
	HttpCurlClient::init();
	stream_receiver.start();
	out_ts_file.append(boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time())).append(".ts");
	open_file = fopen(out_ts_file.data(), "ab");
	stream_receiver.subcribe_ts_callback(boost::bind(receive_ts_data,_1,_2, _3));
	this_thread::sleep_for(std::chrono::seconds(60*3));
	stream_receiver.stop();
	if (open_file)
	{
		fclose(open_file);
	}
	HttpCurlClient::uninit();
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
			//stream_receiver.tsCallback(ts_data, 32768);
		}
	}
}