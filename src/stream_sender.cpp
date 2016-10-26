#include "stdafx.h"
#include "stream_sender.h"
StreamSender::StreamSender()
	:udp_sender_ptr(nullptr)\
	, ts_index_(0), tranlate_rate_(0.0)\
	, pcr_front_(0), pcr_end_(0)\
	, is_first_pcr_(false), sender_buffer_remain_size_(sizeof(send_buffer_))\
	, tranlate_interval_time_(0.0), pcr_duration_packet_num_(0), is_exit_(false)\
	, pop_address_(send_buffer_), ts_packet_num_(0), pcr_front_packet_num_(0)\
	, pcr_end_packet_num_(0), ts_send_content_queue_(32382), ts_packet_queue_(32832)\
	, push_count_(0), out_ts_file_(nullptr), send_delay_time_(0)
{
    out_file_name_.append(boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time())).append(".ts");
	udp_sender_ptr = std::make_shared<UDPClient>(0, 5000);
	out_ts_file_ = fopen("ts.ts", "ab");
	if (!out_ts_file_)
	{
		vvlog_e("ts file open faile");
	}
	memset(send_buffer_, 0, sizeof(send_buffer_));
}

StreamSender::~StreamSender()
{
	std::cout << "come sender destruct!" << std::endl;
	stop();
	//if (out_ts_file.is_open())
	//{
	//	//out_ts_file.close();
	//}
}

int StreamSender::start()
{
	if (multicast_list_.empty())
	{
		return E_REMOTE_ADDR_EMPTY;
	}
	else
	{
		parse_ts_thrd_.reset(new thread(std::bind(&StreamSender::_do_parse_ts_data, this)));
		send_task_thrd_.reset(new thread(std::bind(&StreamSender::_do_send_task_ext, this)));
		if (parse_ts_thrd_ && send_task_thrd_)
		{
			vvlog_i("start send task success.");
			return E_OK;
		}
		else
		{
			vvlog_e("start send task fail");
			return E_PARAM_ERROR;
		}
	}
}

int StreamSender::stop()
{
	is_exit_ = true;

	if (send_task_thrd_ && send_task_thrd_->joinable())
	{
		send_task_thrd_->join();
	}
	send_task_thrd_ = nullptr;
	if (parse_ts_thrd_ && parse_ts_thrd_->joinable())
	{
		parse_ts_thrd_->join();
	}
	parse_ts_thrd_ = nullptr;
	vvlog_i("send stream stop success.");
	return E_OK;
}

void StreamSender::stream_receive_callback(char *data, const long int &data_len, const bool &is_finished)
{
#pragma region  newmethod3
	//std::cout << "sender ts datalen:" << data_len << std::endl;
	long int tmp_data_len = data_len;
	char* tmp_data = data;
	while (tmp_data && TS_PACKET_LENGTH_STANDARD <= tmp_data_len)
	{
		if (0 < ts_remain_packet_.real_size)
		{
			int cpy_size = TS_PACKET_LENGTH_STANDARD - ts_remain_packet_.real_size;
			memcpy(ts_remain_packet_.content + ts_remain_packet_.real_size, tmp_data, cpy_size);
			ts_remain_packet_.real_size = TS_PACKET_LENGTH_STANDARD;
			if (!ts_packet_queue_.push(ts_remain_packet_))
			{
				vvlog_e("push ts data faile len:" << ts_remain_packet_.real_size);
			}
			//else
			//{
			//	vvlog_i("push ts data success");
			//}
			memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));
			tmp_data = tmp_data + cpy_size;
			tmp_data_len -= cpy_size;
		}
		else
		{
			TS_PACKET_CONTENT ts_data;
			memcpy(ts_data.content, tmp_data, TS_PACKET_LENGTH_STANDARD);
			ts_data.real_size = TS_PACKET_LENGTH_STANDARD;
			if (!ts_packet_queue_.push(ts_data))
			{
				vvlog_e("push ts packet error len:" << ts_data.real_size);
			}
			//else
			//{
			//	vvlog_i("push ts data success");
			//}
			tmp_data = tmp_data + TS_PACKET_LENGTH_STANDARD;
			tmp_data_len -= TS_PACKET_LENGTH_STANDARD;
		}
	}
	if (tmp_data && 0 < tmp_data_len)
	{
		if (0 < ts_remain_packet_.real_size)
		{
			int all_size = ts_remain_packet_.real_size + tmp_data_len;
			if (TS_PACKET_LENGTH_STANDARD <= all_size)
			{
				int cpy_size = TS_PACKET_LENGTH_STANDARD - ts_remain_packet_.real_size;
				memcpy(ts_remain_packet_.content + ts_remain_packet_.real_size, tmp_data, cpy_size);
				ts_remain_packet_.real_size = TS_PACKET_LENGTH_STANDARD;
				tmp_data = tmp_data + cpy_size;
				if (!ts_packet_queue_.push(ts_remain_packet_))
				{
					vvlog_e("push ts packet error, len:" << ts_remain_packet_.real_size);
				}
				//else
				//{
				//	vvlog_i("push ts data success");
				//}
				memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));
				int remain_size = all_size - TS_PACKET_LENGTH_STANDARD;
				if (0 < remain_size)
				{
					memcpy(ts_remain_packet_.content, tmp_data, remain_size);
					ts_remain_packet_.real_size = remain_size;
				}
			}
			else
			{
				memcpy(ts_remain_packet_.content + ts_remain_packet_.real_size, tmp_data, tmp_data_len);
				ts_remain_packet_.real_size += tmp_data_len;
			}
		}
		else
		{
			memcpy(ts_remain_packet_.content, tmp_data, tmp_data_len);
			ts_remain_packet_.real_size = tmp_data_len;
		}
	}
#pragma endregion newmethod3
#pragma region newmethod2
	//char *tmp_data = data;
	//long int tmp_data_length = data_len;

	//_write_content_to_file("ts.ts", data, data_len);
	//if (TS_PACKET_LENGTH_STANDARD > data_len)//数据小于188
	//{
	//	//缓存数据
	//	int remain_space = TS_PACKET_LENGTH_STANDARD - ts_remain_packet_.real_size;
	//	if (0 < remain_space && remain_space >= data_len)//缓存可用并且有足够空间
	//	{
	//		memcpy(ts_remain_packet_.content + ts_remain_packet_.real_size, data, data_len);
	//		ts_remain_packet_.real_size += data_len;
	//	}
	//	else//缓存不够用
	//	{
	//		if (ts_packet_.SetPacket((BYTE*)data))//更新缓存的数据 是ts数据
	//		{
	//			memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));
	//			memcpy(ts_remain_packet_.content + ts_remain_packet_.real_size, data, data_len);
	//		}
	//	}
	//	if (ts_remain_packet_.real_size == TS_PACKET_LENGTH_STANDARD)//缓存了一整包TS
	//	{
	//		ts_packet_queue_.push(ts_remain_packet_);
	//		memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));
	//	}
	//	//清空缓存数据
	//}
	//else
	//{
	//	//查找TS的同步字节
	//	int step_count = 0;
	//	bool b_find_sync = false;
	//	while (tmp_data && TS_PACKET_LENGTH_STANDARD <= tmp_data_length)
	//	{
	//		char *second_sync_flag = tmp_data + TS_PACKET_LENGTH_STANDARD;
	//		if (0 >= ts_remain_packet_.real_size)//缓存中无数据
	//		{
	//			if (ts_packet_.SetPacket((BYTE*)tmp_data))
	//			{
	//				b_find_sync = true;
	//				break;//查找到TS同步头
	//			}
	//		}
	//		if (second_sync_flag)
	//		{
	//			int remain_all_count = step_count + ts_remain_packet_.real_size;
	//			if (ts_packet_.SetPacket((BYTE*)tmp_data)\
	//				&& ts_packet_.SetPacket((BYTE*)second_sync_flag)\
	//				&& remain_all_count >= TS_PACKET_LENGTH_STANDARD)
	//			{
	//				b_find_sync = true;
	//				break;//查找到TS同步头
	//			}
	//		}
	//		else
	//		{
	//			int remain_all_count = step_count + ts_remain_packet_.real_size;
	//			if (ts_packet_.SetPacket((BYTE*)tmp_data)\
	//				&& remain_all_count >= TS_PACKET_LENGTH_STANDARD)
	//			{
	//				b_find_sync = true;
	//				break;//查找到TS同步头
	//			}
	//		}
	//		tmp_data++;
	//		tmp_data_length--;
	//		step_count++;
	//	}
	//	if (!b_find_sync)
	//	{
	//		std::cout << "not find sync bytes" << std::endl;
	//	}
	//	//处理头部多余字节
	//	int head_length = tmp_data - data;
	//	if (0 < head_length)//头部有多余字节
	//	{
	//		if (0 < ts_remain_packet_.real_size)//缓存中有数据
	//		{
	//			//缓存数据
	//			int remain_space = TS_PACKET_LENGTH_STANDARD - ts_remain_packet_.real_size;
	//			if (0 < remain_space && remain_space >= head_length)//缓存可用并且有足够空间
	//			{
	//				memcpy(ts_remain_packet_.content + ts_remain_packet_.real_size, data, head_length);
	//				ts_remain_packet_.real_size += head_length;
	//			}
	//			else//缓存不够用
	//			{
	//				if (ts_packet_.SetPacket((BYTE*)data))//更新缓存的数据 是ts数据
	//				{
	//					memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));
	//					if (TS_PACKET_LENGTH_STANDARD >= head_length)
	//					{
	//						memcpy(ts_remain_packet_.content, data, head_length);
	//						ts_remain_packet_.real_size = head_length;
	//					}
	//					else
	//						std::cout << "head more data length:" << head_length << std::endl;
	//				}
	//			}
	//			if (ts_remain_packet_.real_size == TS_PACKET_LENGTH_STANDARD)//缓存了一整包TS
	//			{
	//				ts_packet_queue_.push(ts_remain_packet_);
	//				memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));
	//			}
	//		}
	//	}
	//	//处理连续TS数据
	//	while (tmp_data && TS_PACKET_LENGTH_STANDARD < tmp_data_length)
	//	{
	//		if (ts_packet_.SetPacket((BYTE*)tmp_data))
	//		{
	//			TS_PACKET_CONTENT ts_data;
	//			memcpy(ts_data.content, tmp_data, TS_PACKET_LENGTH_STANDARD);
	//			ts_data.real_size = TS_PACKET_LENGTH_STANDARD;
	//			ts_packet_queue_.push(ts_data);
	//			//vvlog_i("push ts packet count:" << ++push_count_);
	//			tmp_data = tmp_data + TS_PACKET_LENGTH_STANDARD;
	//			tmp_data_length -= TS_PACKET_LENGTH_STANDARD;
	//		}
	//		else
	//		{
	//			//vvlog_e("ts packet error!!!");
	//			cout << "ts packet error!!!" << std::endl;
	//			tmp_data++;//TS同步字节出错
	//			tmp_data_length--;
	//		}
	//	}
	//	//尾部有多余字节
	//	if (tmp_data && 0 < tmp_data_length)
	//	{
	//		//缓存数据
	//		int tail_length = tmp_data_length;
	//		int remain_space = TS_PACKET_LENGTH_STANDARD - ts_remain_packet_.real_size;
	//		if (0 < remain_space && remain_space >= tail_length)//缓存可用并且有足够空间
	//		{
	//			memcpy(ts_remain_packet_.content + ts_remain_packet_.real_size, tmp_data, tail_length);
	//			ts_remain_packet_.real_size += tail_length;
	//		}
	//		else//缓存不够用
	//		{
	//			if (ts_packet_.SetPacket((BYTE*)tmp_data))//更新缓存的数据 是ts数据
	//			{
	//				memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));

	//				if (TS_PACKET_LENGTH_STANDARD >= tail_length)
	//				{
	//					memcpy(ts_remain_packet_.content, tmp_data, tail_length);
	//					ts_remain_packet_.real_size = tail_length;
	//				}
	//				else
	//					std::cout << "head more data length:" << head_length << std::endl;
	//			}
	//		}
	//		if (ts_remain_packet_.real_size == TS_PACKET_LENGTH_STANDARD)//缓存了一整包TS
	//		{
	//			ts_packet_queue_.push(ts_remain_packet_);
	//			memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));
	//		}
	//		//清空缓存数据
	//	}
	//}
#pragma endregion newmethod2
#pragma region newmethod1
	//char out_one_ts[1024] = { 0 };
	//int inner_ts_index = 0;
	//sprintf(out_one_ts, "ts_%d_%d.ts", ts_index_, inner_ts_index);
	//ts_index_++;
	////_write_content_to_file(out_one_ts, data, data_len);
	//_write_content_to_file("ts.ts", data, data_len);
	////return;
	//BYTE *src_data = (BYTE*)data;
	//char *src_data_s = data;
	////vvlog_i("sender come here");
	//cout << "sender come here" << std::endl;
	//long int tmp_data_len = data_len;
	//int more_data_len = 0;//收到数据不是TS头的数据大小

	//if (TS_PACKET_LENGTH_STANDARD > data_len)//收到的数据不足188 添加到remain中
	//{
	//	_write_content_to_file(out_one_ts, data, data_len);
	//	inner_ts_index++;
	//	sprintf(out_one_ts, "ts_%d_%d.ts", ts_index_, inner_ts_index);
	//	if (TS_PACKET_LENGTH_STANDARD >= ts_remain_packet_.real_size + data_len)
	//	{
	//		memcpy(ts_remain_packet_.content+ts_remain_packet_.real_size, data, tmp_data_len);
	//		ts_remain_packet_.real_size = ts_remain_packet_.real_size + tmp_data_len;
	//	}
	//	else
	//	{
	//		cout << "receive ts error!!!" << std::endl;
	//	}
	//	if (ts_remain_packet_.real_size == TS_PACKET_LENGTH_STANDARD)
	//	{
	//		ts_packet_queue_.push(ts_remain_packet_);
	//		memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));
	//	}
	//	else
	//	{
	//		std::cout << "receive error!!!" << std::endl;
	//	}
	//	return;
	//}

	//if (0 < ts_remain_packet_.real_size)//有数据残留
	//{
	//	_write_content_to_file(out_one_ts, (char*)src_data, data_len);
	//	inner_ts_index++;
	//	sprintf(out_one_ts, "ts_%d_%d.ts", ts_index_, inner_ts_index);
	//	int whole_data_len = ts_remain_packet_.real_size + more_data_len;
	//	while (!ts_packet_.SetPacket((BYTE*)data)\
	//		||!ts_packet_.SetPacket((BYTE*)data + TS_PACKET_LENGTH_STANDARD)\
	//		||TS_PACKET_LENGTH_STANDARD > whole_data_len)
	//		//如果不是ts同步字节 或者 此字节188后不是同步字节 或者 内容小于188 进行再次查找
	//	{
	//		data++;
	//		tmp_data_len--;
	//		more_data_len++;
	//		whole_data_len = ts_remain_packet_.real_size + more_data_len;
	//	}
	//	//vvlog_i("sender come here 1");
	//	std::cout << "sender come here 1" << std::endl;
	//	if (TS_PACKET_LENGTH_STANDARD == whole_data_len)
	//	{
	//		char *cpy_src = ts_remain_packet_.content + ts_remain_packet_.real_size;
	//		ts_remain_packet_.real_size = TS_PACKET_LENGTH_STANDARD;
	//		memcpy(cpy_src, (char*)src_data, more_data_len);
	//		ts_packet_queue_.push(ts_remain_packet_);
	//		//vvlog_i("push ts packet count:" << ++push_count_);
	//	}
	//	else
	//	{
	//		//vvlog_e("one ts is not complete so ignore it");
	//		std::cout << "one ts is not complete so ignore it" << std::endl;
	//	}
	//	memset(&ts_remain_packet_, 0, sizeof(ts_remain_packet_));//清空数据残留
	//}
	//while (data && TS_PACKET_LENGTH_STANDARD <= tmp_data_len)
	//{
	//	if (ts_packet_.SetPacket((BYTE*)data))
	//	{
	//		TS_PACKET_CONTENT ts_data;
	//		memcpy(ts_data.content, data, TS_PACKET_LENGTH_STANDARD);
	//		ts_data.real_size = TS_PACKET_LENGTH_STANDARD;
	//		ts_packet_queue_.push(ts_data);
	//		//vvlog_i("push ts packet count:" << ++push_count_);
	//		data = data + TS_PACKET_LENGTH_STANDARD;
	//		tmp_data_len -= TS_PACKET_LENGTH_STANDARD;
	//	}
	//	else
	//	{
	//		//vvlog_e("ts packet error!!!");
	//		cout << "ts packet error!!!" << std::endl;
	//	}
	//}
	//if (data && 0 < tmp_data_len);
	//{
	//	_write_content_to_file(out_one_ts, (char*)src_data, data_len);
	//	inner_ts_index++;
	//	sprintf(out_one_ts, "ts_%d_%d.ts", ts_index_, inner_ts_index);
	//	if (ts_packet_.SetPacket((BYTE*)data))
	//	{
	//		if (TS_PACKET_LENGTH_STANDARD >= ts_remain_packet_.real_size + tmp_data_len)
	//		{
	//			memcpy(ts_remain_packet_.content+ts_remain_packet_.real_size, data, tmp_data_len);
	//			ts_remain_packet_.real_size = ts_remain_packet_.real_size + tmp_data_len;
	//		}
	//	}
	//	else
	//	{
	//		//剩下的数据没有同步字节 丢弃
	//	}
	//}

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
	//vvlog_i("receive data_len:" << data_len << "buffer len" << sender_buffer_.get_data_length());
	while (data && TS_PACKET_LENGTH_STANDARD <= tmp_data_length)
	{
		if (ts_packet_.SetPacket((BYTE*)data))//是TS包头
		{
			sender_buffer_.push_to_buffer((char*)data, TS_PACKET_LENGTH_STANDARD);
			ts_packet_num_++;
			WORD pid = ts_packet_.Get_PID();
			if (ts_packet_.Get_PCR_flag())
			{
				//vvlog_i("ts pcr packet num:" << ts_packet_num_\
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
						//vvlog_i("push send data packetnum:" << packet_num << "bufferlen:" << sender_buffer_.get_data_length());
						_push_data_to_ts_queue(sender_buffer_.get_data()\
							, pcr_front_packet_num_ + pcr_duration_packet_num_
							, tranlate_interval_time_);
					}
					else
					{
						//vvlog_i("push send data packetnum:" << pcr_duration_packet_num_<< "bufferlen:" << sender_buffer_.get_data_length());
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
		//vvlog_i("remain data len:" << tmp_data_length << "data:" << data);
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
						//vvlog_i("push send data packetnum:" << packet_num << "bufferlen:" << sender_buffer_.get_data_length());
						_push_data_to_ts_queue(sender_buffer_.get_data()\
							, pcr_front_packet_num_ + pcr_duration_packet_num_
							, tranlate_interval_time_);
					}
					else
					{
						//vvlog_i("push send data packetnum:" << pcr_duration_packet_num_<< "bufferlen:" << sender_buffer_.get_data_length());
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
			//vvlog_e("remain data is not ts data ignore it!!!");
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
	v_lock(lk, multicast_mtx_);
	if (multicast_list_.end() == multicast_list_.find(id_str))
	{
		IPPORTPAIR ip_port;
		ip_port.ip = remote_addr;
		ip_port.port = port;
		multicast_list_.insert(std::make_pair(id_str, ip_port));
		return true;
	}
	return false;
	//std::map<string, UDPClientPtr>::iterator iter = sender_clients_list_.find(id_str);
	//auto udp_client = std::make_shared<UDPClient>(0, 5, remote_addr, port);
	//if (iter == sender_clients_list_.end())
	//{
	//	sender_clients_list_.insert(std::make_pair(id_str, udp_client));
	//	return true;
	//}
	//else
	//{
	//	return false;
	//}

}

bool StreamSender::del_sender_address(const string &remote_addr, const int &port)
{
	v_lock(lk, multicast_mtx_);
	char id_str[25] = { 0 };
	sprintf(id_str, "%s:%d", remote_addr.data(), port);
	multicast_list_.erase(id_str);
	return true;
}

void StreamSender::set_delay_time(const int &delay_time_ms)
{
	send_delay_time_ = delay_time_ms;
}

void StreamSender::set_receive_url(const string &url)
{
	receive_url_ = url;
}

void StreamSender::_push_ts_data_to_send_queue(char *data, const long int &data_len, const int &need_time/*bytes:(188*7):micro*/)
{
	char *tmp_data = data;
	long int tmp_data_len = data_len;
	TS_SEND_CONTENT ts_send_content;
	while (TS_SEND_SIZE <= tmp_data_len)
	{
		memcpy(ts_send_content.content, tmp_data, TS_SEND_SIZE);
		ts_send_content.need_time = need_time;
		ts_send_content.real_size = TS_SEND_SIZE;
		if (!ts_send_content_queue_.push(ts_send_content))
		{
			vvlog_e("push ts send content to queue faile!!");
		}
		tmp_data_len -=TS_SEND_SIZE;
		tmp_data = tmp_data + TS_SEND_SIZE;
		memset(&ts_send_content, 0, sizeof(ts_send_content));
	}
	if (0 < tmp_data_len)
	{
		memcpy(ts_send_content.content, tmp_data, tmp_data_len);
		ts_send_content.need_time = (tmp_data_len*need_time) / (TS_SEND_SIZE);
		ts_send_content.real_size = tmp_data_len;
		if (!ts_send_content_queue_.push(ts_send_content))
		{
			vvlog_e("push ts send content to queue fail!");
		}
		
		memset(&ts_send_content, 0, sizeof(ts_send_content));
	}
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
//	while (1)
//	{
//		TS_PACKET_CONTENT ts_data;
//		if (is_exit_)
//		{
//			break;
//		}
//		while (ts_packet_queue_.pop(ts_data))
//		{
//			//vvlog_i("receive datalen:" << ts_data.real_size << "data:" << std::hex << ts_data.content);
//			//if (out_ts_file_)
//			//{
//			//	fwrite(ts_data.content, 1, ts_data.real_size, out_ts_file_);
//			//}
//			//_write_content_to_file("end.ts", ts_data.content, ts_data.real_size);
//			if (ts_packet_.SetPacket((BYTE*)ts_data.content))
//			{
//				PCR packet_pcr = ts_packet_.Get_PCR();
//				WORD pid = ts_packet_.Get_PID();
//				//stream_buffer_.pushToBuffer(ts_data.content, ts_data.real_size);
//				int result = sender_buffer_.push_to_buffer(ts_data.content, ts_data.real_size);
//				if (E_OK != result)
//				{
//					vvlog_e("push to sender buff fail, error:" << result);
//				}
//				pcr_duration_packet_num_++;
//				if (INVALID_PCR != packet_pcr)
//				{
//					if (!is_first_pcr_)//
//					{
//						is_first_pcr_ = true;
//						pcr_front_ = pcr_end_ = ts_packet_.Get_PCR();
//					}
//					else
//					{
//						pcr_front_ = pcr_end_;
//						pcr_end_ = ts_packet_.Get_PCR();
//						if (pcr_end_ < pcr_front_)
//						{
//							max_pcr_ = pcr_front_;
//							pcr_front_ = 0;
//						}
//						tranlate_rate_ = (double)8*pcr_duration_packet_num_*TS_PACKET_LENGTH_STANDARD / (double)(pcr_end_ - pcr_front_);
//						tranlate_interval_time_ = TS_SEND_SIZE *8/ (tranlate_rate_ * 1000) - 1;//发送188*7需要的时间
//                        _do_send_ts_data(sender_buffer_.get_data(), sender_buffer_.get_data_length(), tranlate_interval_time_);
//						sender_buffer_.reset_buffer();
//                        //_do_send_ts_data(stream_buffer_.data(), stream_buffer_.data_len(), tranlate_interval_time_);
//					}
//				}
//			}
//			else
//			{
//				vvlog_e("invaild packet data:" << ts_data.content);
//			}
//			//for (auto item : multicast_list_)
//			/*{
//				udp_sender_ptr->write_ext(ts_data.content\
//					, ts_data.real_size\
//					, 1\
//					, item.second.ip\
//					, item.second.port);
//			}*/
//			//_write_content_to_file("ts_translate.ts", ts_data.content, ts_data.real_size);
//		}
//		this_thread::sleep_for(std::chrono::microseconds(1));
//	}
//	return;
//#pragma region test
//	/*while (1)
//	{
//		char ts_data_tmp[1024];
//		stream_buffer_.pop_data(ts_data_tmp, sender_buffer_remain_size_, TS_PACKET_LENGTH_STANDARD);
//		memset(ts_data_tmp, 0, sizeof(ts_data_tmp));
//	}*/
//#pragma endregion test
//	char *pop_address = send_buffer_;
//	while (1)
//	{
//		if (stream_buffer_.is_empty())
//		{
//			this_thread::sleep_for(std::chrono::microseconds(1));
//			continue;
//		}
//		if (is_exit_)
//		{
//			break;
//		}
//		//pop的数据是完整的一个TS包 如果不是会存在问题 每次pop的数据量太小导致接受端的阻塞
//		if (pop_address)
//		{
//			if (stream_buffer_.pop_data(pop_address, sender_buffer_remain_size_, TS_PACKET_LENGTH_STANDARD))
//			{
//				if (ts_packet_.SetPacket((BYTE*)pop_address))
//				{
//					WORD pid = ts_packet_.Get_PID();
//					if (ts_packet_.Get_PCR_flag())
//					{
//						if (!is_first_pcr_)
//						{
//							pcr_front_ = pcr_end_ = ts_packet_.Get_PCR();
//							is_first_pcr_ = true;
//						}
//						else
//						{
//							pcr_front_ = pcr_end_;
//							pcr_end_ = ts_packet_.Get_PCR();
//							if (pcr_end_ < pcr_front_)
//							{
//								max_pcr_ = pcr_front_;
//								pcr_front_ = 0;
//							}
//							tranlate_rate_ = (double)8*pcr_duration_packet_num_*TS_PACKET_LENGTH_STANDARD / (double)(pcr_end_ - pcr_front_);
//							tranlate_interval_time_ = TS_SEND_SIZE *8/ (tranlate_rate_ * 1000) - 1;//发送188*7需要的时间
//							//将数据发送出去
//							_do_send_ts_data(send_buffer_, TS_PACKET_LENGTH_STANDARD*(pcr_duration_packet_num_+1), tranlate_interval_time_);
//							cout << "pid:" << pid << "ts_cout:" << pcr_duration_packet_num_ << "rate:" << tranlate_rate_ << "pcr front:" << pcr_front_ << "pcr end:" << pcr_end_ << std::endl;
//							//初始化相关变量
//							pcr_duration_packet_num_ = 0;
//							sender_buffer_remain_size_ = sizeof(send_buffer_);
//							memset(send_buffer_, 0, sizeof(send_buffer_));//清空buffer
//							pop_address = send_buffer_;
//							continue;
//						}
//					}
//				}
//				sender_buffer_remain_size_ -= TS_PACKET_LENGTH_STANDARD;
//				if (sender_buffer_remain_size_ < TS_PACKET_LENGTH_STANDARD)
//				{
//					cout << "no remain size to store ts data" << std::endl;
//					assert(true);
//				}
//				pcr_duration_packet_num_++;
//				pop_address += TS_PACKET_LENGTH_STANDARD;
//			}
//
//		}
//		this_thread::sleep_for(std::chrono::microseconds(1));
//	}
//
}

void StreamSender::_do_send_task_ext()
{
	while (1)
	{
		if (is_exit_)
		{
			break;
		}
		//if (!ts_send_content_queue_.empty())
		//{
		//	std::call_once(delay_flag_, [&]()
		//	{
		//		this_thread::sleep_for(std::chrono::milliseconds(send_delay_time_));
		//	});
		//}
		TS_SEND_CONTENT ts_send_content;
		while (ts_send_content_queue_.pop(ts_send_content))
		{
			//std::call_once(delay_flag_, [&]() 
			//{
			//	this_thread::sleep_for(std::chrono::milliseconds(send_delay_time_));
			//});
			for (auto item : multicast_list_)
			{
				//std::cout << "multi send size:" << ts_send_content.real_size\
					<< "time:" << ts_send_content.need_time << std::endl;
				udp_sender_ptr->write_ext(ts_send_content.content\
					, ts_send_content.real_size\
					, ts_send_content.need_time\
					, item.second.ip\
					, item.second.port);
			}
		}
		this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

void StreamSender::_do_send_ts_data(char * data, const long int & data_len, const int &time)
{
	long int tmp_data_len = data_len;
	char *tmp_data = data;
	while (TS_SEND_SIZE < tmp_data_len)
	{
		//udp_sender_ptr->write(tmp_data, TS_SEND_SIZE, time);
        for (auto item : multicast_list_)
        {
            udp_sender_ptr->write_ext(tmp_data\
                , TS_SEND_SIZE\
                , time\
                , item.second.ip\
                , item.second.port);
			//cout << "udp multicast send to ip:" << item.second.ip\
			<< "port:" << item.second.port\
			<< "data size:" << TS_SEND_SIZE << "remain data size:" << tmp_data_len << std::endl;
        }
		tmp_data += TS_SEND_SIZE;
		tmp_data_len -= TS_SEND_SIZE;
	}
	if (0 < tmp_data_len)
	{
		int send_time = time*tmp_data_len / TS_SEND_SIZE;
		//udp_sender_ptr->write(tmp_data, tmp_data_len, send_time);
        for (auto item : multicast_list_)
        {
            udp_sender_ptr->write_ext(tmp_data\
                , tmp_data_len\
                , send_time\
                , item.second.ip\
                , item.second.port);
			//cout << "udp multicast send to ip:" << item.second.ip\
				<< "port:" << item.second.port\
				<< "data size:" << tmp_data_len << "remain data size:"<< tmp_data_len << std::endl;
        }
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
	FILE *open_file = fopen(file_name.c_str(), "ab");
	if (open_file)
	{
		if (10 < length)
		{
			if (*data == 0x00 && *(data + 1) == 0x00 && *(data + 3) == 0x00 && *(data + 5) == 0x00 && *(data + 9) == 0)
			{
				assert(false);
			}
		}
		int write_size = fwrite(data, sizeof(char), length, open_file);
		fflush(open_file);
		if (0 > write_size)
		{
			assert(false);
		}
		fclose(open_file);
	}
	else
	{
#ifdef _WIN32
		std::cout << file_name <<" file open error:" << GetLastError() << std::endl;
#else
		perror("file open error");
#endif // _DEBUG
		assert(false);
	}
	return;
	fstream out_file;
	char out_ts[1024] = { 0 };
	sprintf(out_ts, file_name.data());
	out_file.open(out_ts, ios::out | ios::binary | ios::app);
	if (out_file.is_open())
	{
		out_file.write(data, length);
		out_file.flush();
		out_file.close();
	}
	else
	{
		std::cout << "open file bad " << std::endl;
	}

}

void StreamSender::_do_parse_ts_data()
{
	StreamSenderBuffer sender_buffer;
	CTsPacket ts_packet;
	TS_PACKET_CONTENT ts_data;
	long int ts_packet_num = 0;

	long int pcr_first_packet_num = 0;//找到第一包PCR收到的数据包
	long int pcr_second_packet_num = 0;//找到第二包PCR收到的数据包
	long int pcr_duration_packet_num = 0;
	long int pcr_front = 0;
	long int pcr_end = 0;
	long int max_pcr = 0;
	bool is_find_first_pcr = false;
	while (1)
	{
		if (is_exit_)
		{
			break;
		}
		while (ts_packet_queue_.pop(ts_data))
		{
			//int write_count = fwrite(ts_data.content, 1, ts_data.real_size, out_ts_file_);
			//if (0 >= write_count)
			//{
			//	vvlog_e("write file error");
			//}
			if (ts_packet.SetPacket((BYTE*)ts_data.content))
			{
				PCR packet_pcr = ts_packet.Get_PCR();
				WORD pid = ts_packet.Get_PID();
				int result = sender_buffer.push_to_buffer(ts_data.content, ts_data.real_size);
				if (E_OK != result)
				{
					vvlog_e("push ts data to sender buff fail, error:" << result);
				}
				ts_packet_num++;
				if (INVALID_PCR != packet_pcr)
				{
					if (!is_find_first_pcr)//
					{
						is_find_first_pcr = true;
						pcr_front = pcr_end = packet_pcr;
						pcr_first_packet_num = ts_packet_num;
					}
					else
					{
						pcr_front = pcr_end;
						pcr_end = packet_pcr;
						if (pcr_end < pcr_front)
						{
							max_pcr = pcr_front;
							pcr_front = 0;
						}
						pcr_second_packet_num = ts_packet_num;
						pcr_duration_packet_num = pcr_second_packet_num - pcr_first_packet_num;
						long int system_clock_reference = 27;
						double tranlate_rate = (double)(8 * pcr_duration_packet_num*TS_PACKET_LENGTH_STANDARD*system_clock_reference) / (pcr_end - pcr_front);
						double tranlate_interval_time = (double)(188 * 7 * 8) / (long)(1000 * tranlate_rate);//发送188*7需要的时间
						//std::cout << "first_pcr:" << pcr_front << "pcr_end:" << pcr_end\
							<< "rate:" << tranlate_rate << "time:" << tranlate_interval_time << std::endl;
						pcr_first_packet_num = pcr_second_packet_num = ts_packet_num = 0;
						_push_ts_data_to_send_queue(sender_buffer.get_data(), sender_buffer.get_data_length(), tranlate_interval_time);
						sender_buffer.reset_buffer();
					}
				}
			}
			else
			{
				std::cout << "invaild packet, url:" << receive_url_ << std::endl;
				//vvlog_e("it is a invalid packet!! receiverurl:"<<receive_url_ << "len:" << ts_data.real_size);
			}
			memset(&ts_data, 0, sizeof(ts_data));
		}
		this_thread::sleep_for(std::chrono::nanoseconds(1));
	}
}

/*void StreamSender::_push_data_to_ts_queue(char*data, const int &data_len, const int &send_time)
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
*/

