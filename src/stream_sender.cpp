#include "stdafx.h"
#include "stream_sender.h"
StreamSender::StreamSender()
	:udp_sender_ptr(nullptr)\
	, is_ts_task_exit_(false)\
	, out_ts_file_(nullptr), send_delay_time_(0)
{
    out_file_name_.append(boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time())).append(".ts");
	udp_sender_ptr = std::make_shared<UDPClient>(0, 5000);
#if ENABLE_OUTFILE
	out_ts_file_ = fopen(out_file_name_.data(), "ab");
#endif
}


StreamSender::StreamSender(StreamReceiverPtr receiver)
	:udp_sender_ptr(nullptr)\
	, is_ts_task_exit_(false), out_ts_file_(nullptr)\
	, send_delay_time_(0), stream_receiver_(receiver)
{
    out_file_name_.append(boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time())).append(".ts");
	udp_sender_ptr = std::make_shared<UDPClient>(0, 5000);
#if ENABLE_OUTFILE
	out_ts_file_ = fopen(out_file_name_.data(), "ab");
#endif
}

StreamSender::~StreamSender()
{
	std::cout << "come sender destruct!" << std::endl;
	is_ts_task_exit_ = true;
	if (out_ts_file_)
	{
		fclose(out_ts_file_);
	}
}

int StreamSender::start()
{
	if (multicast_list_.empty())
	{
		return E_REMOTE_ADDR_EMPTY;
	}
	else
	{
		send_task_thrd_.reset(new thread(std::bind(&StreamSender::_do_send_task_ext, this)));
		if (send_task_thrd_)
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
	is_ts_task_exit_ = true;
	if (send_task_thrd_ && send_task_thrd_->joinable())
	{
		send_task_thrd_->join();
	}
	send_task_thrd_ = nullptr;
	vvlog_i("send stream stop success.");
	return E_OK;
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

void StreamSender::_do_send_task_ext()
{
	TSSendQueueType *ts_data_queue = nullptr;
	stream_receiver_->get_send_queue(ts_data_queue);
	while (1)
	{
		if (is_ts_task_exit_)
		{
			break;
		}
		if (stream_receiver_ && ts_data_queue)
		{
			TS_SEND_CONTENT ts_send_content;
			int send_count = ts_data_queue->consume_all([&](TS_SEND_CONTENT &ts_data_item)
			{
				v_lock(lk, multicast_mtx_);
				for (auto item : multicast_list_)
				{
					udp_sender_ptr->write_ext(ts_data_item.content\
						, ts_data_item.real_size\
						, ts_data_item.need_time\
						, item.second.ip\
						, item.second.port);
				}
			});
			//if (0 < send_count)
			//{
			//	vvlog_i("send multi udp cnt:" << send_count << "url:" << receive_url_);
			//}
		}
		//	while (ts_data_queue->pop(ts_send_content))
		//	{
		//		//std::call_once(delay_flag_, [&]() 
		//		//{
		//		//	this_thread::sleep_for(std::chrono::milliseconds(send_delay_time_));
		//		//});
		//		//v_lock(lk, multicast_mtx_);
		//		for (auto item : multicast_list_)
		//		{
		//			//std::cout << "multi send size:" << ts_send_content.real_size\
		//				<< "time:" << ts_send_content.need_time << std::endl;
		//			udp_sender_ptr->write_ext(ts_send_content.content\
		//				, ts_send_content.real_size\
		//				, ts_send_content.need_time\
		//				, item.second.ip\
		//				, item.second.port);
		//		}
		//		memset(&ts_send_content, 0, sizeof(ts_send_content));
		//	}
		//}
		//this_thread::sleep_for(std::chrono::nanoseconds(1));
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
