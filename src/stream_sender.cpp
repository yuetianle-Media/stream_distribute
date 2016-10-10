#include "stdafx.h"
#include "stream_sender.h"
StreamSender::StreamSender()
	:udp_sender_ptr(nullptr), stream_buffer_(1024*1024*4)\
	, ts_index_(0), tranlate_rate_(0.0)\
	, pcr_front_(0), pcr_end_(0)\
	, is_first_pcr_(false), sender_buffer_remain_size_(sizeof(send_buffer_))\
	, tranlate_interval_time_(0.0), pcr_duration_packet_num_(0), is_exit_(false)\
	, pop_address_(send_buffer_), ts_packet_num_(0), pcr_front_packet_num_(0)\
	, pcr_end_packet_num_(0), ts_send_content_queue_(12800), ts_packet_queue_(32832)\
	, push_count_(0)
{
	memset(send_buffer_, 0, sizeof(send_buffer_));
}

StreamSender::~StreamSender()
{

}

int StreamSender::start()
{
	if (sender_clients_list_.empty())
	{
		return E_REMOTE_ADDR_EMPTY;
	}
	else
	{
		for (auto item : sender_clients_list_)
		{
			if (item.second)
			{
				item.second->connect();
			}
		}
		send_task_thrd_.reset(new thread(std::bind(&StreamSender::_do_send_task, this)));
		//send_task_thrd_.reset(new thread(std::bind(&StreamSender::_do_send_task_ext, this)));
		return E_OK;
	}
}

int StreamSender::stop()
{
	is_exit_ = true;

	for (auto item : sender_clients_list_)
	{
	}
	if (send_task_thrd_ && send_task_thrd_->joinable())
	{
		send_task_thrd_->join();
	}
	send_task_thrd_ = nullptr;
	return E_OK;
}

void StreamSender::stream_receive_callback(char *data, const long int &data_len)
{
#pragma region newmethod1
	char out_one_ts[1024] = { 0 };
	sprintf(out_one_ts, "ts_%d.ts", ts_index_);
	ts_index_++;
	//_write_content_to_file(out_one_ts, data, data_len);
	_write_content_to_file("ts.ts", data, data_len);
	//return;
	BYTE *src_data = (BYTE*)data;
	//vvlog_i("sender come here");
	cout << "sender come here" << std::endl;
	long int tmp_data_len = data_len;
	int more_data_len = 0;//收到数据不是TS头的数据大小

	if (TS_PACKET_LENGTH_STANDARD > data_len)//收到的数据不足188 添加到remain中
	{
		if (TS_PACKET_LENGTH_STANDARD >= ts_remain_packet_.real_size + data_len)
		{
			memcpy(ts_remain_packet_.content+ts_remain_packet_.real_size, data, tmp_data_len);
			ts_remain_packet_.real_size = ts_remain_packet_.real_size + tmp_data_len;
		}
		else
		{
			cout << "receive ts error!!!" << std::endl;
		}
		if (ts_remain_packet_.real_size == TS_PACKET_LENGTH_STANDARD)
		{
			ts_packet_queue_.push(ts_remain_packet_);
			memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));
		}
		return;
	}

	if (0 < ts_remain_packet_.real_size)//有数据残留
	{
		int whole_data_len = ts_remain_packet_.real_size + more_data_len;
		while (!ts_packet_.SetPacket((BYTE*)data)\
			||!ts_packet_.SetPacket((BYTE*)data + TS_PACKET_LENGTH_STANDARD)\
			||TS_PACKET_LENGTH_STANDARD > whole_data_len)
			//如果不是ts同步字节 或者 此字节188后不是同步字节 或者 内容小于188 进行再次查找
		{
			data++;
			tmp_data_len--;
			more_data_len++;
			whole_data_len = ts_remain_packet_.real_size + more_data_len;
		}
		//vvlog_i("sender come here 1");
		std::cout << "sender come here 1" << std::endl;
		if (TS_PACKET_LENGTH_STANDARD == whole_data_len)
		{
			char *cpy_src = ts_remain_packet_.content + ts_remain_packet_.real_size;
			ts_remain_packet_.real_size = TS_PACKET_LENGTH_STANDARD;
			memcpy(cpy_src, src_data, more_data_len);
			ts_packet_queue_.push(ts_remain_packet_);
			//vvlog_i("push ts packet count:" << ++push_count_);
		}
		else
		{
			//vvlog_e("one ts is not complete so ignore it");
			std::cout << "one ts is not complete so ignore it" << std::endl;
		}
		memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));//清空数据残留
	}
	while (data && TS_PACKET_LENGTH_STANDARD <= tmp_data_len)
	{
		if (ts_packet_.SetPacket((BYTE*)data))
		{
			TS_PACKET_CONTENT ts_data;
			memcpy(ts_data.content, data, TS_PACKET_LENGTH_STANDARD);
			ts_data.real_size = TS_PACKET_LENGTH_STANDARD;
			ts_packet_queue_.push(ts_data);
			//vvlog_i("push ts packet count:" << ++push_count_);
			data = data + TS_PACKET_LENGTH_STANDARD;
			tmp_data_len -= TS_PACKET_LENGTH_STANDARD;
		}
		else
		{
			//vvlog_e("ts packet error!!!");
			cout << "ts packet error!!!" << std::endl;
		}
	}
	if (data && 0 < tmp_data_len);
	{
		if (TS_PACKET_LENGTH_STANDARD >= ts_remain_packet_.real_size + tmp_data_len)
		{
			memcpy(ts_remain_packet_.content+ts_remain_packet_.real_size, data, tmp_data_len);
			ts_remain_packet_.real_size = ts_remain_packet_.real_size + tmp_data_len;
		}
	}

#pragma endregion newmethod1
#ifdef TEST
#pragma region

	//std::cout << "tsdata index:" << ts_index_ << "data_len" << data_len << std::endl;
	fstream out_file;
	fstream out_one_file;
	char out_ts[1024] = { 0 };
	//char out_one_ts[1024] = { 0 };
	//sprintf(out_one_ts, "ts_%d.ts", ts_index_);
	sprintf(out_ts, "ts.ts");
	out_file.open(out_ts, ios::out | ios::binary | ios::app);
	out_file.write(data, data_len);
	out_file.close();

	//out_one_file.open(out_one_ts, ios::out | ios::binary | ios::app);
	//out_one_file.write(data, data_len);
	//out_one_file.close();
	ts_index_++;
	//return;
#pragma endregion writefile
#pragma region NewMethod
	BYTE* data = (BYTE*)data;
	long int tmp_data_length = data_len;
	vvlog_i("receive data_len:" << data_len << "buffer len" << sender_buffer_.get_data_length());
	while (data && TS_PACKET_LENGTH_STANDARD <= tmp_data_length)
	{
		if (ts_packet_.SetPacket((BYTE*)data))//是TS包头
		{
			sender_buffer_.push_to_buffer((char*)data, TS_PACKET_LENGTH_STANDARD);
			ts_packet_num_++;
			WORD pid = ts_packet_.Get_PID();
			if (ts_packet_.Get_PCR_flag())
			{
				vvlog_i("ts pcr packet num:" << ts_packet_num_\
<< "pcr_front:" << pcr_front_ << "pcr_end:" << pcr_end_);
				if (!is_first_pcr_)
				{
					pcr_front_ = pcr_end_ = ts_packet_.Get_PCR();
					is_first_pcr_ = true;
					pcr_front_packet_num_ = ts_packet_num_;
				}
				else//将数据推送到队列当中
				{
					//sender_buffer_.push_to_buffer((char*)tmp_data, TS_PACKET_LENGTH_STANDARD);
					pcr_front_ = pcr_end_;
					pcr_end_ = ts_packet_.Get_PCR();
					if (pcr_end_ < pcr_front_)
					{
						max_pcr_ = pcr_front_;
						pcr_front_ = 0;
					}
					pcr_end_packet_num_ = ts_packet_num_;
					pcr_duration_packet_num_ = pcr_end_packet_num_ - pcr_front_packet_num_;
					tranlate_rate_ = (double)8*pcr_duration_packet_num_*TS_PACKET_LENGTH_STANDARD / (double)(pcr_end_ - pcr_front_);
					tranlate_interval_time_ = TS_SEND_SIZE *8/ (tranlate_rate_ * 1000) - 1;//发送188*7需要的时间
					if (0 < pcr_front_packet_num_)
					{
						int packet_num = pcr_front_packet_num_ + pcr_duration_packet_num_;
						vvlog_i("push send data packetnum:" << packet_num << "bufferlen:" << sender_buffer_.get_data_length());
						_push_data_to_ts_queue(sender_buffer_.get_data()\
							, pcr_front_packet_num_ + pcr_duration_packet_num_
							, tranlate_interval_time_);
					}
					else
					{
						vvlog_i("push send data packetnum:" << pcr_duration_packet_num_<< "bufferlen:" << sender_buffer_.get_data_length());
						_push_data_to_ts_queue(sender_buffer_.get_data()\
							, pcr_duration_packet_num_\
							, tranlate_interval_time_);
					}
					sender_buffer_.reset_buffer();
					pcr_front_packet_num_ = 0;
					pcr_end_packet_num_ = 0;
					pcr_duration_packet_num_ = 0;
					ts_packet_num_ = 0;
				}
			}
			data = data + TS_PACKET_LENGTH_STANDARD;
			tmp_data_length -= TS_PACKET_LENGTH_STANDARD;
		}
		else
		{
			sender_buffer_.push_to_buffer((char*)data, 1);
			tmp_data_length--;
			data++;
		}
	}
	//the last data is less than 188
	if (data && 0 < tmp_data_length)
	{
		vvlog_i("remain data len:" << tmp_data_length << "data:" << data);
		if (ts_packet_.SetPacket((BYTE*)data))//是TS数据
		{
			sender_buffer_.push_to_buffer((char*)data, tmp_data_length);
			ts_packet_num_++;
			if (ts_packet_.Get_PCR_flag())
				if (!is_first_pcr_)
				{
					pcr_front_ = pcr_end_ = ts_packet_.Get_PCR();
					is_first_pcr_ = true;
					pcr_front_packet_num_ = ts_packet_num_;
				}
				else//将数据推送到队列当中
				{
					pcr_front_ = pcr_end_;
					pcr_end_ = ts_packet_.Get_PCR();
					if (pcr_end_ < pcr_front_)
					{
						max_pcr_ = pcr_front_;
						pcr_front_ = 0;
					}
					pcr_end_packet_num_ = ts_packet_num_;
					pcr_duration_packet_num_ = pcr_end_packet_num_ - pcr_front_packet_num_;
					tranlate_rate_ = (double)8*pcr_duration_packet_num_*TS_PACKET_LENGTH_STANDARD / (double)(pcr_end_ - pcr_front_);
					tranlate_interval_time_ = TS_SEND_SIZE *8/ (tranlate_rate_ * 1000) - 1;//发送188*7需要的时间
					if (0 < pcr_front_packet_num_)
					{
						int packet_num = pcr_front_packet_num_ + pcr_duration_packet_num_;
						vvlog_i("push send data packetnum:" << packet_num << "bufferlen:" << sender_buffer_.get_data_length());
						_push_data_to_ts_queue(sender_buffer_.get_data()\
							, pcr_front_packet_num_ + pcr_duration_packet_num_
							, tranlate_interval_time_);
					}
					else
					{
						vvlog_i("push send data packetnum:" << pcr_duration_packet_num_<< "bufferlen:" << sender_buffer_.get_data_length());
						_push_data_to_ts_queue(sender_buffer_.get_data()\
							, pcr_duration_packet_num_\
							, tranlate_interval_time_);
					}
					sender_buffer_.reset_buffer();
					pcr_front_packet_num_ = 0;//有一包不完整的还没有推送
					pcr_end_packet_num_ = 0;
					pcr_duration_packet_num_ = 0;
					ts_packet_num_ = 0;
				}
		}
		else
		{
			vvlog_e("remain data is not ts data ignore it!!!");
		}
	}
	else
	{
		assert(true);
	}

#pragma endregion NewMethod
#else
	//stream_buffer_.pushToBuffer(data, data_len);
#endif
	
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
	while (1)
	{
		TS_PACKET_CONTENT ts_data;
		if (ts_packet_queue_.empty())
			this_thread::sleep_for(chrono::microseconds(1));
		if (!ts_packet_queue_.is_lock_free())
			vvlog_e("queue is not lcok free!!!");
		while (ts_packet_queue_.pop(ts_data))
		{
			_write_content_to_file("ts_translate.ts", ts_data.content, ts_data.real_size);
		}
	}
	return;
#pragma region test
	/*while (1)
	{
		char ts_data_tmp[1024];
		stream_buffer_.pop_data(ts_data_tmp, sender_buffer_remain_size_, TS_PACKET_LENGTH_STANDARD);
		memset(ts_data_tmp, 0, sizeof(ts_data_tmp));
	}*/
#pragma endregion test
	char *pop_address = send_buffer_;
	while (1)
	{
		if (stream_buffer_.is_empty())
		{
			this_thread::sleep_for(std::chrono::microseconds(1));
			continue;
		}
		if (is_exit_)
		{
			break;
		}
		//pop的数据是完整的一个TS包 如果不是会存在问题 每次pop的数据量太小导致接受端的阻塞
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
							for (auto udp_client_ptr : sender_clients_list_)
							{
								udp_sender_ptr = udp_client_ptr.second;
								_do_send_ts_data(send_buffer_, TS_PACKET_LENGTH_STANDARD*(pcr_duration_packet_num_+1), tranlate_interval_time_);
								//auto udp_client = udp_client_ptr.second;
								//std::thread thrd([this, udp_client]() {
								//	_do_send_ts_data(udp_client, send_buffer_, TS_PACKET_LENGTH_STANDARD*(pcr_duration_packet_num_ + 1), tranlate_interval_time_);
								//});
							}
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
					assert(true);
				}
				pcr_duration_packet_num_++;
				pop_address += TS_PACKET_LENGTH_STANDARD;
			}
			
		}
		this_thread::sleep_for(std::chrono::microseconds(1));
	}

}

void StreamSender::_do_send_task_ext()
{
	while (1)
	{
		TS_SEND_CONTENT ts_send_content;
		if (ts_send_content_queue_.empty())
			this_thread::sleep_for(chrono::microseconds(1));
		if (!ts_send_content_queue_.is_lock_free())
		{
			vvlog_e("queue is not lockfree!!!!");
		}
		while (ts_send_content_queue_.pop(ts_send_content))
		{
			for (auto &udp_client_ptr : sender_clients_list_)
			{
				//cout << "dataLen:" << ts_send_content.real_size << std::endl;
				//vvlog_i("start send data datalen:" << ts_send_content.real_size);
				if (udp_client_ptr.second)
				{
					_write_content_to_file("ts_translate.ts"\
						, ts_send_content.content\
						, ts_send_content.real_size);
					//udp_client_ptr.second->write(ts_send_content.content, ts_send_content.real_size, ts_send_content.need_time);
				}
			}
		}
	}
}

void StreamSender::_do_send_ts_data(char * data, const long int & data_len, const int &time)
{
	long int tmp_data_len = data_len;
	char *tmp_data = data;
	while (TS_SEND_SIZE < tmp_data_len)
	{
		udp_sender_ptr->write(tmp_data, TS_SEND_SIZE, time);
		tmp_data += TS_SEND_SIZE;
		tmp_data_len -= TS_SEND_SIZE;
		cout << "udp multicast send to ip:" << udp_sender_ptr->remote_ip()\
			<< "port:" << udp_sender_ptr->remote_port()\
			<< "data size:" << TS_SEND_SIZE << "remain data size:" << tmp_data_len << std::endl;
	}
	if (0 < data_len)
	{
		int send_time = time*tmp_data_len / TS_SEND_SIZE;
		udp_sender_ptr->write(tmp_data, tmp_data_len, send_time);
		cout << "udp multicast send to ip:" << udp_sender_ptr->remote_ip()\
			<< "port:" << udp_sender_ptr->remote_port()\
			<< "data size:" << tmp_data_len << "remain data size:"<< tmp_data_len << std::endl;
	}
}

void StreamSender::_do_send_ts_data(UDPClientPtr udp_client, char* data, const long int &data_len, const int &time)
{
	long int tmp_data_len = data_len;
	char *tmp_data = data;
	while (TS_SEND_SIZE < data_len)
	{
		cout << "udp multicast send to ip:" << udp_client->remote_ip()\
			<< "port:" << udp_client->remote_port()\
			<< "data size:" << data_len << std::endl;
		udp_client->write(data, TS_SEND_SIZE, time);
		tmp_data += TS_SEND_SIZE;
		tmp_data_len -= TS_SEND_SIZE;
	}
	if (0 < data_len)
	{
		cout << "udp multicast send to ip:" << udp_client->remote_ip()\
			<< "port:" << udp_client->remote_port()\
			<< "data size:" << data_len << std::endl;
		int send_time = time*data_len / TS_SEND_SIZE;
		udp_client->write(data, TS_SEND_SIZE, time);
	}

}

void StreamSender::_write_content_to_file(const string &file_name, const char* data, const int &length)
{
	fstream out_file;
	char out_ts[1024] = { 0 };
	sprintf(out_ts, file_name.data());
	out_file.open(out_ts, ios::out | ios::binary | ios::app);
	out_file.write(data, length);
	out_file.close();
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

void StreamSender::_push_data_to_ts_queue(char*data, const int &ts_packet_num, const int &send_time)
{
	TS_SEND_CONTENT ts_send_content;
	int tmp_packet_num = ts_packet_num;
	char *tmp_data = data;
	while (7 <= tmp_packet_num)
	{
		memcpy(ts_send_content.content, tmp_data, TS_SEND_SIZE);
		ts_send_content.need_time = send_time;
		ts_send_content.real_size = TS_SEND_SIZE;
		ts_send_content_queue_.push(ts_send_content);
		tmp_packet_num -=7;
		data = data + TS_SEND_SIZE;
	}
	if (0 < tmp_packet_num)
	{
		memcpy(ts_send_content.content, tmp_data, tmp_packet_num*TS_PACKET_LENGTH_STANDARD);
		ts_send_content.need_time = (tmp_packet_num*TS_PACKET_LENGTH_STANDARD*send_time) / (TS_SEND_SIZE);
		ts_send_content.real_size = tmp_packet_num*TS_PACKET_LENGTH_STANDARD;
		ts_send_content_queue_.push(ts_send_content);
	}
		
}

