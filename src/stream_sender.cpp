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
	//udp_sender_ptr->set_reuse(true);
	udp_sender_ptr->set_noblock(true);
#if ENABLE_OUTFILE
	out_ts_file_ = fopen(out_file_name_.data(), "ab");
#endif
}

StreamSender::StreamSender(TESTReceiverPtr receiver, const string &local_ip/*="127.0.0.1"*/)
	 :udp_sender_ptr(nullptr)\
	, is_ts_task_exit_(false), out_ts_file_(nullptr)\
	, send_delay_time_(0), ts_stream_receiver_(receiver)\
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
		//send_task_thrd_.reset(new thread(std::bind(&StreamSender::_do_send_task_ext, this)));
		send_task_thrd_.reset(new std::thread(std::bind(&StreamSender::_do_send_task, this)));
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
	//TSSendSpscQueueType *ts_data_queue = nullptr;
	//stream_receiver_->get_send_queue(ts_data_queue);

	TSSendUnlimitQueueType *ts_data_queue = nullptr;
	if (stream_receiver_)
	{
		//stream_receiver_->get_unlimit_queue(ts_data_queue);
	}
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
	std::cout << "send ts task pid:" << std::this_thread::get_id() << std::endl;
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
			v_lock(lk, multicast_mtx_);
			for (auto item : multicast_list_)
			{
				udp_sender_ptr->write_ext(ts_send_content.content\
					, ts_send_content.real_size\
					, item.second.ip\
					, item.second.port);
			}
			//all_need_time += ts_send_content.need_time;
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
					std::this_thread::sleep_for(std::chrono::microseconds(all_need_time - all_run_time));
					//Sleep((all_need_time - all_run_time) / 1000);
				}
				sleep_packet_count = SLEEP_COUNT;
				all_run_time = 0;
				all_need_time = 0;
			}
		}
		//this_thread::yield();
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
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

void StreamSender::_do_send_task()
{
	TSSendUnlimitQueueType *ts_data_queue = nullptr;
	TSSendSpscQueueType *ts_spsc_data_queue = nullptr;
	ts_comm_send_queue ts_comm_data_queue;
	if (stream_receiver_)
	{
		//stream_receiver_->get_unlimit_queue(ts_data_queue);
		stream_receiver_->get_send_queue(ts_spsc_data_queue);
		TSSendSpscSDQueueTypePtr sd_send_queue		=nullptr;
		TSSendSpscHDQueueTypePtr hd_send_queue	    = nullptr;
		TSSendSpscFHDQueueTypePtr fhd_send_queue	= nullptr;
		TSSendSpsc2KQueueTypePtr ts_2k_send_queue	= nullptr;
		TSSendSpsc4KQueueTypePtr ts_4k_send_queue	= nullptr;
		if (stream_receiver_->get_ts_send_sd_queue(sd_send_queue))
		{
			_doing_send_task(sd_send_queue);
		}
		else if (stream_receiver_->get_ts_send_hd_queue(hd_send_queue))
		{
			_doing_send_task(hd_send_queue);
		}
		else if (stream_receiver_->get_ts_send_fhd_queue(fhd_send_queue))
		{
			_doing_send_task(fhd_send_queue);
		}
		else if (stream_receiver_->get_ts_send_2k_queue(ts_2k_send_queue))
		{
			_doing_send_task(ts_2k_send_queue);
		}
		else if (stream_receiver_->get_ts_send_4k_queue(ts_4k_send_queue))
		{
			_doing_send_task(ts_4k_send_queue);
		}
	}
	if (ts_stream_receiver_)
	{
		ts_stream_receiver_->get_send_queue(ts_spsc_data_queue);
		TSSendSpscSDQueueTypePtr sd_send_queue		=nullptr;
		TSSendSpscHDQueueTypePtr hd_send_queue	    = nullptr;
		TSSendSpscFHDQueueTypePtr fhd_send_queue	= nullptr;
		TSSendSpsc2KQueueTypePtr ts_2k_send_queue	= nullptr;
		TSSendSpsc4KQueueTypePtr ts_4k_send_queue	= nullptr;
		if (ts_stream_receiver_->get_ts_send_sd_queue(sd_send_queue))
		{
			_doing_send_task(sd_send_queue);
		}
		else if (ts_stream_receiver_->get_ts_send_hd_queue(hd_send_queue))
		{
			_doing_send_task(hd_send_queue);
		}
		else if (ts_stream_receiver_->get_ts_send_fhd_queue(fhd_send_queue))
		{
			_doing_send_task(fhd_send_queue);
		}
		else if (ts_stream_receiver_->get_ts_send_2k_queue(ts_2k_send_queue))
		{
			_doing_send_task(ts_2k_send_queue);
		}
		else if (ts_stream_receiver_->get_ts_send_4k_queue(ts_4k_send_queue))
		{
			_doing_send_task(ts_4k_send_queue);
		}
	}
	return;
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
	vvlog_i("doing send ts task cmd:" << receive_url_ << "pid:" << std::this_thread::get_id());

	int64_t start_time = 0;
	int64_t cur_time = 0;
	PCR cur_pcr = 0;
	PCR first_pcr = 0;
	bool is_first = false;
	long int delay_time = send_delay_time_;
	while (1)
	{
		if (is_ts_task_exit_)
		{
			break;
		}
		PCR cur_cout = cur_pcr - first_pcr; 
		cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		int64_t time_cout = cur_time - start_time;
		//if (time_cout < cur_cout / 27000)
		if (time_cout < (cur_cout / 90))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		//if(ts_data_queue->pop(ts_send_content))
		if (ts_spsc_data_queue->pop(ts_send_content))
		{
			//std::cout << "pop success" << std::endl;
			//if (0 < delay_time)
			//{
			//	std::call_once(delay_flag_, [&]() 
			//	{
			//		std::this_thread::sleep_for(std::chrono::seconds(delay_time));
			//	});
			//}
			if (ts_send_content.is_real_pcr && !is_first)
			{
				start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				cur_pcr = ts_send_content.cur_pcr;
				first_pcr = ts_send_content.cur_pcr;
				is_first = true;
			}
			if (ts_send_content.is_real_pcr)
			{
				//if ((ts_send_content.cur_pcr - cur_pcr) / 90 > 10000 || ts_send_content.cur_pcr < cur_pcr)
				//if ((ts_send_content.cur_pcr - cur_pcr) / 90 > 100 || ts_send_content.cur_pcr < cur_pcr)
				//std::cout << "pcr duration:" << (ts_send_content.cur_pcr - cur_pcr) << "time:" <<(ts_send_content.cur_pcr - cur_pcr)/90 << std::endl;
				if ((ts_send_content.cur_pcr - cur_pcr) / 90 > 1000 || ts_send_content.cur_pcr < cur_pcr)/*<< 时间间隔大于1S*/
				{
					first_pcr = ts_send_content.cur_pcr;
					start_time = cur_time;
				}
				cur_pcr = ts_send_content.cur_pcr;
			}
			v_lock(lk, multicast_mtx_);
			for (auto item : multicast_list_)
			{
				udp_sender_ptr->write_ext(ts_send_content.content\
					, ts_send_content.real_size\
					, item.second.ip\
					, item.second.port);
				//std::cout << "multi send len" << ts_send_content.real_size << std::endl;
			}
		}
		else
		{
			//std::cout << "pop fail" << std::endl;
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

}
