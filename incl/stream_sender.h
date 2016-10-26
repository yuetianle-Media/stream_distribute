#ifndef _STREAM_SENDER_H_
#define _STREAM_SENDER_H_
#pragma once
#include <boost/lockfree/queue.hpp>
#include "udpclient.h"
#include "stream_buffer.h"
#include "stream_sender_buffer.h"
#include "tspacket.h"

#ifndef TS_SEND_SIZE
//此值得设置和MTU相关
#define TS_SEND_SIZE 188*7
#endif
typedef struct TS_SEND_CONTENT
{
	char content[TS_SEND_SIZE];
	int real_size;
	double need_time;
	TS_SEND_CONTENT()
		:real_size(0), need_time(0)
	{
		memset(content, 0, sizeof(TS_SEND_SIZE));
	}
}TSSENDCONTENT;

typedef struct TSPACKETCONTENT
{
	char content[TS_PACKET_LENGTH_STANDARD];
	int real_size;
	TSPACKETCONTENT()
	{
		memset(content, 0, sizeof(TSPACKETCONTENT));
	}
}TS_PACKET_CONTENT;//一个ts包

/*typedef struct TS_BASIC_CONTENT
{
	int start_index;
	int packet_num;
};
*/

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
class StreamSender :public std::enable_shared_from_this<StreamSender>
{
public:
	StreamSender();
	~StreamSender();

	int start();
	int stop();

	bool add_sender_address(const string &remote_addr, const int &port);
	bool del_sender_address(const string &remote_addr, const int &port);

	void set_delay_time(const int &delay_time_ms);
	void set_receive_url(const string &url);
	void stream_receive_callback(char *data, const long int &data_len, const bool &is_finished);
private:

	void _write_content_to_file(const string &file_name, const char* data, const int &length);
	void _do_parse_ts_data();
	//void _parse_ts_data(char* data, const int data_len);

	//void _push_data_to_ts_queue(char*data, const int &data_len, const int &send_time);
	void _push_ts_data_to_send_queue(char *data, const long int &data_len, const int &need_time/*bytes:(188*7):micro*/);
	void _do_send_task();
	void _do_send_task_ext();
	void _do_send_ts_data(char* data, const long int &data_len, const int &time);
	void _do_send_ts_data(UDPClientPtr udp_client, char* data, const long int &data_len, const int &time);

	UDPClientPtr udp_sender_ptr;
	//StreamBuffer stream_buffer_;
	CTsPacket ts_packet_;
	long ts_index_;
	char send_buffer_[188 * 7 * 200];//<* 发送的缓存区 至少存在两个PCRTS包>
	int sender_buffer_remain_size_;

	std::atomic<bool> is_first_pcr_;
	PCR pcr_front_;
	PCR pcr_end_;
	PCR max_pcr_;
	int ts_packet_num_;//接收到的ts数据包.
	int pcr_front_packet_num_;//第一个PCR收到的数据包 unit:188字节.
	int pcr_end_packet_num_;//第二个PCR收到的数据包 unit:188字节.
	int pcr_duration_packet_num_;//两个PCR之间的ts包数
	double tranlate_rate_;
	double tranlate_interval_time_;
	std::map<string, UDPClientPtr> sender_clients_list_;/*<< (ip:port):udpclient example "224.0.1.20:8000"*/

	std::mutex multicast_mtx_;
	std::map<std::string, IPPORTPAIR> multicast_list_;
	std::shared_ptr<std::thread> send_task_thrd_;
	std::shared_ptr<std::thread> parse_ts_thrd_;
	std::atomic<bool> is_exit_;
	char *pop_address_;
	StreamSenderBuffer sender_buffer_;
	boost::lockfree::queue<TSSENDCONTENT, boost::lockfree::fixed_sized<true>> ts_send_content_queue_;

	boost::lockfree::queue<TS_PACKET_CONTENT, boost::lockfree::fixed_sized<true>> ts_packet_queue_;//32K数据缓存大小
	TS_PACKET_CONTENT ts_remain_packet_;
	int push_count_;
	std::mutex ts_send_mtx_;
    std::string out_file_name_;
	FILE * out_ts_file_;
	std::atomic<int> send_delay_time_;
	std::once_flag delay_flag_;

	std::string receive_url_;
};

typedef std::shared_ptr<StreamSender> StreamSenderPtr;
#endif
