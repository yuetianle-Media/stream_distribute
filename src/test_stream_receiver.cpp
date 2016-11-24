#include "stdafx.h"
#include "test_stream_receiver.h"


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
	StreamReceiverPtr stream_receiver = std::make_shared<StreamReceiver>(url);
	HttpCurlClient::init();
	stream_receiver->start();
	out_ts_file.append(boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time())).append(".ts");
	open_file = fopen(out_ts_file.data(), "ab");
	TSSendSpscQueueType *ts_send_queue = nullptr;
	bool is_finished = false;
	auto consume_task(new std::thread([&]()
	{
		TSSendUnlimitQueueType *unlimint_queue = nullptr;
		while (1)
		{
			if (is_finished)
			{
				break;
			}
			if (stream_receiver->get_send_queue(ts_send_queue))
			{
				ts_send_queue->consume_all([](TSSENDCONTENT item) {
					if (0 < item.real_size)
					{
						write_content_to_file("ts.ts", item.content, item.real_size);
					}
				});
			}
		}
	}));
	//stream_receiver.subcribe_ts_callback(boost::bind(receive_ts_data,_1,_2, _3));
	//Sleep(5000);
	std::this_thread::sleep_for(std::chrono::seconds(60*3));
	is_finished = true;
	consume_task->join();
	stream_receiver->stop();
	if (open_file)
	{
		fclose(open_file);
	}
	HttpCurlClient::uninit();
};

void test_async_task()
{
	std::vector<std::shared_ptr<std::thread>> task_list;
	std::async(std::launch::async, [&]() {
		std::shared_ptr<std::thread> task = std::make_shared<std::thread>([&] {
			while (1)
			{
				vvlog_i("task one this is cout");
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		});
		task_list.push_back(task);
	});

	std::async(std::launch::async, [&]() {
		std::shared_ptr<std::thread> task = std::make_shared<std::thread>([&] {
			while (1)
			{
				vvlog_i("task two this is cout");
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		});
		task_list.push_back(task);
	});
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

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