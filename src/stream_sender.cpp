#include "stdafx.h"
#include "stream_sender.h"
StreamSender::StreamSender()
	:udp_sender_ptr(nullptr), stream_buffer_(1024*1024*4)\
	, ts_index_(0), tranlate_rate_(0.0)\
	, pcr_front_(0), pcr_end_(0)\
	, is_first_pcr_(false), sender_buffer_remain_size_(sizeof(send_buffer_))\
	, tranlate_interval_time_(0.0), pcr_duration_packet_num_(0)
{
	memset(send_buffer_, 0, sizeof(send_buffer_));
	send_task_thrd_.reset(new thread(std::bind(&StreamSender::_do_send_task, this)));
}

StreamSender::~StreamSender()
{

}

void StreamSender::stream_receive_callback(char *data, const int &data_len)
{
	std::cout << "tsdata index:" << ts_index_ << "data_len" << data_len << std::endl;
	fstream out_file;
	char out_ts[1024] = { 0 };
	//sprintf(out_ts, "ts_%d", ts_index_);
	sprintf(out_ts, "ts.ts");
	out_file.open(out_ts, ios::out | ios::binary | ios::app);
	out_file.write(data, data_len);
	out_file.close();
	ts_index_++;
	stream_buffer_.pushToBuffer(data, data_len);
}


void StreamSender::_do_send_task()
{
	char *pop_address = send_buffer_;
	while (1)
	{
		if (stream_buffer_.is_empty())
		{
			this_thread::sleep_for(std::chrono::microseconds(1));
			continue;;
		}
		//pop的数据是完整的一个TS包 如果不是会存在问题
		if (pop_address)
		{
			if (stream_buffer_.pop_data(pop_address, sender_buffer_remain_size_, TS_PACKET_LENGTH_STANDARD))
			{
				if (ts_packet_.SetPacket((BYTE*)pop_address))
				{
					WORD pid = ts_packet_.Get_PID();
					if (ts_packet_.Get_PCR_flag())
					{
						if (!is_first_pcr_)
						{
							pcr_front_ = ts_packet_.Get_PCR();
							is_first_pcr_ = true;
						}
						else
						{
							pcr_front_ = pcr_end_;
							pcr_end_ = ts_packet_.Get_PCR();
							tranlate_rate_ = 8*pcr_duration_packet_num_*TS_PACKET_LENGTH_STANDARD / (pcr_end_ - pcr_front_);
							tranlate_interval_time_ = TS_SEND_SIZE *8/ tranlate_rate_ * 1000 - 1;//发送188*7需要的时间
							//将数据发送出去
							pcr_duration_packet_num_ = 0;
							sender_buffer_remain_size_ = sizeof(send_buffer_);
							memset(send_buffer_, 0, sizeof(send_buffer_));//清空buffer
							pop_address = send_buffer_;
							return;
						}
					}
					cout << "pid:" << pid << "ts_cout:" << pcr_duration_packet_num_ << "pcr front:" << pcr_front_ << "pcr end:" << pcr_end_;
				}
				sender_buffer_remain_size_ -= TS_PACKET_LENGTH_STANDARD;
				pcr_duration_packet_num_++;
				pop_address += TS_PACKET_LENGTH_STANDARD;
			}
			
		}
		this_thread::sleep_for(std::chrono::microseconds(1));
	}

}
void StreamSender::_parse_ts_data()
{
	char *pop_address = send_buffer_;
	while (1)
	{
		if (stream_buffer_.is_empty())
		{
			this_thread::sleep_for(std::chrono::microseconds(1));
			continue;;
		}
		//pop的数据是完整的一个TS包 如果不是会存在问题
		if (pop_address)
		{
			stream_buffer_.pop_data(pop_address, sender_buffer_remain_size_, TS_PACKET_LENGTH_STANDARD);
			if (ts_packet_.SetPacket((BYTE*)pop_address))
			{
				if (ts_packet_.Get_PCR_flag())
				{
					WORD pid = ts_packet_.Get_PID();
					if (!is_first_pcr_)
					{
						pcr_front_ = ts_packet_.Get_PCR();
						is_first_pcr_ = true;
					}
					else
					{
						pcr_front_ = pcr_end_;
						pcr_end_ = ts_packet_.Get_PCR();
						tranlate_rate_ = 8*pcr_duration_packet_num_*TS_PACKET_LENGTH_STANDARD / (pcr_end_ - pcr_front_);
						tranlate_interval_time_ = TS_SEND_SIZE *8/ tranlate_rate_ * 1000 - 1;//发送188*7需要的时间
						//将数据发送出去
						pcr_duration_packet_num_ = 0;
						sender_buffer_remain_size_ = sizeof(send_buffer_);
						memset(send_buffer_, 0, sizeof(send_buffer_));//清空buffer
						pop_address = send_buffer_;
						return;
					}
					cout << "pid:" << pid << "ts_cout:" << pcr_duration_packet_num_\
<< "pcr front:" << pcr_front_ << "pcr end:" << pcr_end_;
				}
			}
			pcr_duration_packet_num_++;
			pop_address += TS_PACKET_LENGTH_STANDARD;
		}
		this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

