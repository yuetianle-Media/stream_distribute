#ifndef _STREAM_SENDER_H_
#define _STREAM_SENDER_H_
#pragma once
#include "udpclient.h"
#include "stream_buffer.h"
#include "tspacket.h"

#ifndef TS_SEND_SIZE
//此值得设置和MTU相关
#define TS_SEND_SIZE 188*7
#endif
class StreamSender
{
public:
	StreamSender(const string &remote_ip, const int &remote_port);
	~StreamSender();

	void stream_receive_callback(char *data, const long int &data_len);
	bool add_sender_address(const string &remote_addr, const int &port);
	bool del_sender_address(const string &remote_addr, const int &port);
private:
	void _parse_ts_data();

	void _do_send_task();
	void _do_send_ts_data(char* data, const long int &data_len, const int &time);

	UDPClientPtr udp_sender_ptr;
	StreamBuffer stream_buffer_;
	CTsPacket ts_packet_;
	long ts_index_;
	char send_buffer_[188 * 7 * 200];//<* 发送的缓存区 至少存在两个PCRTS包>
	int sender_buffer_remain_size_;
	std::atomic<bool> is_first_pcr_;
	PCR pcr_front_;
	PCR pcr_end_;
	PCR max_pcr_;
	int pcr_duration_packet_num_;//两个PCR之间的ts包数
	double tranlate_rate_;
	double tranlate_interval_time_;
	std::map<string, UDPClientPtr> sender_clients_list_;/*<< (ip:port):udpclient example "224.0.1.20:8000"*/
	std::shared_ptr<std::thread> send_task_thrd_;
};
#endif