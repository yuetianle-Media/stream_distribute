#include "stdafx.h"
#include "stream_sender.h"
StreamSender::StreamSender(const string &remote_ip, const int &remote_port)
	:udp_sender_ptr(nullptr), stream_buffer_(1024*1024*4)\
	, ts_index_(0), tranlate_rate_(0.0)\
	, pcr_front_(0), pcr_end_(0)\
	, is_first_pcr_(false), sender_buffer_remain_size_(sizeof(send_buffer_))\
	, tranlate_interval_time_(0.0), pcr_duration_packet_num_(0)
{
	memset(send_buffer_, 0, sizeof(send_buffer_));
	send_task_thrd_.reset(new thread(std::bind(&StreamSender::_do_send_task, this)));
	udp_sender_ptr.reset(new UDPClient(0, 5, remote_ip, remote_port));
	if (udp_sender_ptr)
		udp_sender_ptr->connect();
}

StreamSender::~StreamSender()
{

}

void StreamSender::stream_receive_callback(char *data, const long int &data_len)
{
	//std::cout << "tsdata index:" << ts_index_ << "data_len" << data_len << std::endl;
	//fstream out_file;
	//fstream out_one_file;
	//char out_ts[1024] = { 0 };
	//char out_one_ts[1024] = { 0 };
	//sprintf(out_one_ts, "ts_%d.ts", ts_index_);
	//sprintf(out_ts, "ts.ts");
	//out_file.open(out_ts, ios::out | ios::binary | ios::app);
	//out_file.write(data, data_len);
	//out_file.close();

	//out_one_file.open(out_one_ts, ios::out | ios::binary | ios::app);
	//out_one_file.write(data, data_len);
	//out_one_file.close();
	//ts_index_++;
	stream_buffer_.pushToBuffer(data, data_len);
}

bool StreamSender::add_sender_address(const string &remote_addr, const int &port)
{
	char id_str[25] = { 0 };
	sprintf(id_str, "%s:%d", remote_addr.data(), port);
	std::map<string, UDPClientPtr>::iterator iter = sender_clients_list_.find(id_str);
	auto udp_client = std::make_shared<UDPClient>(0, 5, remote_addr, port);
	if (iter == sender_clients_list_.end())
	{
		sender_clients_list_.insert(std::make_pair(id_str, udp_client));
		return true;
	}
	else
	{
		return false;
	}
	
}

bool StreamSender::del_sender_address(const string &remote_addr, const int &port)
{
	return true;
}

//void StreamSender::_do_send_task()
//{
//	char *pop_address = send_buffer_;
//	while (1)
//	{
//		if (stream_buffer_.is_empty())
//		{
//			this_thread::sleep_for(std::chrono::microseconds(1));
//			continue;;
//		}
//		pop的数据是完整的一个TS包 如果不是会存在问题
//		if (pop_address)
//		{
//			if (stream_buffer_.pop_data(pop_address, sender_buffer_remain_size_, TS_PACKET_LENGTH_STANDARD))
//			{
//				if (ts_packet_.SetPacket((BYTE*)pop_address))
//				{
//					WORD pid = ts_packet_.Get_PID();
//					if (ts_packet_.Get_PCR_flag())
//					{
//						cout << "pid:" << pid << "ts_cout:" << pcr_duration_packet_num_ << "pcr:" << ts_packet_.Get_PCR() << std::endl;
//						pcr_duration_packet_num_ = 0;
//					}
//					memset(pop_address, 0, sizeof(send_buffer_));
//				}
//				pcr_duration_packet_num_++;
//			}
//			
//		}
//		this_thread::sleep_for(std::chrono::microseconds(1));
//	}
//
//
//}

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
							pcr_front_ = pcr_end_ = ts_packet_.Get_PCR();
							is_first_pcr_ = true;
						}
						else
						{
							pcr_front_ = pcr_end_;
							pcr_end_ = ts_packet_.Get_PCR();
							if (pcr_end_ < pcr_front_)
							{
								max_pcr_ = pcr_front_;
								pcr_front_ = 0;
							}
							tranlate_rate_ = (double)8*pcr_duration_packet_num_*TS_PACKET_LENGTH_STANDARD / (double)(pcr_end_ - pcr_front_);
							tranlate_interval_time_ = TS_SEND_SIZE *8/ (tranlate_rate_ * 1000) - 1;//发送188*7需要的时间
							//将数据发送出去
							_do_send_ts_data(send_buffer_, TS_PACKET_LENGTH_STANDARD*(pcr_duration_packet_num_+1), tranlate_interval_time_);
							cout << "pid:" << pid << "ts_cout:" << pcr_duration_packet_num_ << "rate:" << tranlate_rate_ << "pcr front:" << pcr_front_ << "pcr end:" << pcr_end_ << std::endl;
							//初始化相关变量
							pcr_duration_packet_num_ = 0;
							sender_buffer_remain_size_ = sizeof(send_buffer_);
							memset(send_buffer_, 0, sizeof(send_buffer_));//清空buffer
							pop_address = send_buffer_;
							continue;
						}
					}
				}
				sender_buffer_remain_size_ -= TS_PACKET_LENGTH_STANDARD;
				if (sender_buffer_remain_size_ < TS_PACKET_LENGTH_STANDARD)
				{
					cout << "no remain size to store ts data" << std::endl;
				}
				pcr_duration_packet_num_++;
				pop_address += TS_PACKET_LENGTH_STANDARD;
			}
			
		}
		this_thread::sleep_for(std::chrono::microseconds(1));
	}

}
void StreamSender::_do_send_ts_data(char * data, const long int & data_len, const int &time)
{
	long int tmp_data_len = data_len;
	char *tmp_data = data;
	while (TS_SEND_SIZE < data_len)
	{
		cout << "udp multicast send to ip:" << udp_sender_ptr->remote_ip()\
			<< "port:" << udp_sender_ptr->remote_port()\
			<< "data size:" << data_len << std::endl;
		udp_sender_ptr->write(data, TS_SEND_SIZE, time);
		tmp_data += TS_SEND_SIZE;
		tmp_data_len -= TS_SEND_SIZE;
	}
	if (0 < data_len)
	{
		cout << "udp multicast send to ip:" << udp_sender_ptr->remote_ip()\
			<< "port:" << udp_sender_ptr->remote_port()\
			<< "data size:" << data_len << std::endl;
		int send_time = time*data_len / TS_SEND_SIZE;
		udp_sender_ptr->write(data, TS_SEND_SIZE, time);
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
<< "pcr front:" << pcr_front_ << "pcr end:" << pcr_end_ <<std::endl;
				}
				if (ts_packet_.Get_adaptation_field_control())
				{
					cout << "pid:" << ts_packet_.Get_PID() << " has adaption field" << std::endl;
				}
			}
			else
			{
				cout << "first:"<< hex << pop_address[0] << "" << std::endl;
			}
			pcr_duration_packet_num_++;
			pop_address += TS_PACKET_LENGTH_STANDARD;
		}
		this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

