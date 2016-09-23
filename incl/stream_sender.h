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
	StreamSender();
	~StreamSender();

	void stream_receive_callback(char *data, const int &data_len);
private:
	void _parse_ts_data();

	void _do_send_task();
	UDPClientPtr udp_sender_ptr;
	StreamBuffer stream_buffer_;
	CTsPacket ts_packet_;
	long ts_index_;
	char send_buffer_[188 * 7 * 50];//<* 发送的缓存区 至少存在两个PCRTS包>
	int sender_buffer_remain_size_;
	std::atomic<bool> is_first_pcr_;
	PCR pcr_front_;
	PCR pcr_end_;
	int pcr_duration_packet_num_;//两个PCR之间的ts包数
	double tranlate_rate_;
	double tranlate_interval_time_;

	std::shared_ptr<std::thread> send_task_thrd_;
};
#endif