#include "stdafx.h"
#include "stream_sender.h"
StreamSender::StreamSender()
	:udp_sender_ptr(nullptr)\
	, is_ts_task_exit_(false)\
	, out_ts_file_(nullptr), send_delay_time_(0)
{
    out_file_name_.append(boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time())).append(".ts");
	udp_sender_ptr = std::make_shared<UDPClient>(0, 5000);
	udp_sender_ptr->resize_send_buffer_size(1024 * 1024 * 1.25);
	udp_sender_ptr->set_reuse(true);
	udp_sender_ptr->set_noblock(true);
#if ENABLE_OUTFILE
	out_ts_file_ = fopen(out_file_name_.data(), "ab");
#endif
}


StreamSender::StreamSender(StreamReceiverPtr receiver, const std::string &local_ip)
	:udp_sender_ptr(nullptr)\
	, is_ts_task_exit_(false), out_ts_file_(nullptr)\
	, send_delay_time_(0), stream_receiver_(receiver)\
	, local_ip_(local_ip)
{
    out_file_name_.append(boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time())).append(".ts");
	if (local_ip_.empty())
	{
		udp_sender_ptr = std::make_shared<UDPClient>(0, 5000);
	}
	else
	{
		udp_sender_ptr = std::make_shared<UDPClient>(0, 5000,local_ip_);
	}
	udp_sender_ptr->resize_send_buffer_size(1024 * 1024 * 1.25);
	udp_sender_ptr->set_reuse(true);
	udp_sender_ptr->set_noblock(true);
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
			send_task_thrd_->detach();
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

void StreamSender::set_local_ip(const string &local_ip)
{
	if (local_ip != local_ip_)
	{

		boost::asio::ip::address_v4 local_interface = boost::asio::ip::address_v4::from_string(local_ip);
		boost::asio::ip::multicast::outbound_interface bound_interface(local_interface);
		boost::system::error_code ec;
		udp_sender_ptr->socket().set_option(bound_interface, ec);
		if (!ec)
		{
			std::cout << "bound success ip:" << local_ip << std::endl;
		}
		local_ip_ = local_ip;
	}
}

void StreamSender::set_receive_url(const string &url)
{
	receive_url_ = url;
}

void StreamSender::_do_send_task_ext()
{
	//TSSendQueueType *ts_data_queue = nullptr;
	TSSendSpscQueueType *ts_data_queue = nullptr;
	stream_receiver_->get_send_queue(ts_data_queue);
	bool is_first_send = false;
	TS_SEND_CONTENT ts_send_content;
	int64_t success_time = 0;
	int64_t last_sleep_time = 0;
	int64_t run_time_count = 0;
	int64_t sleep_time = 0;

	long int sleep_packet_count = SLEEP_COUNT;//可以使用随机数
	int64_t all_run_time = 0;
	int64_t all_need_time = 0;
	int64_t cur_time_count = 0;
	int64_t send_start_time = 0;
	while (1)
	{
		if (is_ts_task_exit_)
		{
			break;
		}
		while (ts_data_queue && ts_data_queue->pop(ts_send_content))
		{
			//send_start_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			//send_start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			send_start_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			for (auto item : multicast_list_)
			{
				udp_sender_ptr->write_ext(ts_send_content.content\
					, ts_send_content.real_size\
					, ts_send_content.need_time\
					, item.second.ip\
					, item.second.port);
			}
			all_need_time += ts_send_content.need_time;
			//success_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			//success_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			success_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			all_run_time += (success_time - send_start_time);
			sleep_packet_count--;
			if (0 == sleep_packet_count)
			{
				if (all_need_time > all_run_time)
				{
					//this_thread::sleep_for(std::chrono::nanoseconds(all_need_time - all_run_time));
					//this_thread::sleep_for(std::chrono::milliseconds(all_need_time - all_run_time));
					this_thread::sleep_for(std::chrono::microseconds(all_need_time - all_run_time));
				}
				sleep_packet_count = SLEEP_COUNT;
				all_run_time = 0;
				all_need_time = 0;
			}
		}
		this_thread::sleep_for(std::chrono::nanoseconds(1));
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
