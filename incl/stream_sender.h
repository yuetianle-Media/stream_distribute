#ifndef _STREAM_SENDER_H_
#define _STREAM_SENDER_H_
#pragma once
#include <boost/lockfree/queue.hpp>
#include "udpclient.h"
#include "stream_buffer.h"
#include "stream_sender_buffer.h"
#include "tspacket.h"
#include "data_types_defs.h"
#include "stream_receiver.h"


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
	std::string receive_url_;

	std::string local_ip_;
};

typedef std::shared_ptr<StreamSender> StreamSenderPtr;
#endif
