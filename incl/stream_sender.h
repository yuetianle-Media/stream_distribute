#ifndef _STREAM_SENDER_H_
#define _STREAM_SENDER_H_
#pragma once

#include "pre_boost_basic.h"
#include "udpclient.h"
#include "stream_buffer.h"
#include "stream_sender_buffer.h"
#include "tspacket.h"
#include "data_types_defs.h"
#include "stream_receiver.h"
#include "ts_test_receiver.h"
using namespace std;
typedef struct IPPORTV
{
	std::string ip;
	int port;
	IPPORTV()
		:port(0)
	{

	}
}IPPORTPAIR;
//#define TEST 1

#define ENABLE_OUTFILE 0
#define SLEEP_COUNT 30
class StreamSender :public std::enable_shared_from_this<StreamSender>
{
public:
	StreamSender();
	StreamSender(StreamReceiverPtr receiver, const string &local_ip="127.0.0.1");
	StreamSender(TESTReceiverPtr receiver, const string &local_ip="127.0.0.1");
	~StreamSender();

	int start();
	int stop();

	bool add_sender_address(const string &remote_addr, const int &port);
	bool del_sender_address(const string &remote_addr, const int &port);

	void set_delay_time(const int &delay_time_ms);
	void set_local_ip(const string &local_ip);
	void set_receive_url(const string &url);
private:

	void _write_content_to_file(const string &file_name, const char* data, const int &length);

	void _do_send_task();

	template <typename T>
	void _doing_send_task(T &data_queue);
	void _do_send_task_ext();

	UDPClientPtr udp_sender_ptr;

	std::mutex multicast_mtx_;
	std::map<std::string, IPPORTPAIR> multicast_list_;

	std::shared_ptr<std::thread> send_task_thrd_;
	std::atomic<bool> is_ts_task_exit_;

    std::string out_file_name_;
	FILE * out_ts_file_;

	std::atomic<int> send_delay_time_;
	std::once_flag delay_flag_;

	StreamReceiverPtr stream_receiver_;
	TESTReceiverPtr ts_stream_receiver_;
	std::string receive_url_;

	std::string local_ip_;
};

template <typename T>
void StreamSender::_doing_send_task(T &data_queue)
{
	TS_SEND_CONTENT ts_send_content;
	vvlog_i("doing send ts task cmd:" << receive_url_ << "pid:" << std::this_thread::get_id());
	int64_t start_time = 0;
	int64_t cur_time = 0;
	PCR cur_pcr = 0;
	PCR first_pcr = 0;
	bool is_first = false;
	long int delay_time = send_delay_time_;
	while (1)
	{
		if (is_ts_task_exit_)
		{
			break;
		}
		PCR cur_cout = cur_pcr - first_pcr; 
		cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		int64_t time_cout = cur_time - start_time;
		if (time_cout < (cur_cout / 90))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		if (data_queue->pop(ts_send_content))
		{
			if (ts_send_content.is_real_pcr && !is_first)
			{
				start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				cur_pcr = ts_send_content.cur_pcr;
				first_pcr = ts_send_content.cur_pcr;
				is_first = true;
			}
			if (ts_send_content.is_real_pcr)
			{
				if ((ts_send_content.cur_pcr - cur_pcr) / 90 > 1000 || ts_send_content.cur_pcr < cur_pcr)/*<< Ê±¼ä¼ä¸ô´óÓÚ1S*/
				{
					first_pcr = ts_send_content.cur_pcr;
					start_time = cur_time;
				}
				cur_pcr = ts_send_content.cur_pcr;
			}
			v_lock(lk, multicast_mtx_);
			for (auto item : multicast_list_)
			{
				udp_sender_ptr->write_ext(ts_send_content.content\
					, ts_send_content.real_size\
					, item.second.ip\
					, item.second.port);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
}
typedef std::shared_ptr<StreamSender> StreamSenderPtr;
#endif
