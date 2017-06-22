#include "stdafx.h"
#include "ts_test_receiver.h"
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>


TESTReceiver::TESTReceiver(const string &url)
	:ts_task_list_(10000), play_stream_duration_(0)\
	, ts_thrd_ptr_(nullptr), parse_ts_thrd_(nullptr)\
	, b_exit_ts_task_(false), b_exit_parse_task_(false)\
	, ts_file_index_(0), uri_(url), receive_ts_buffer_(TS_PACKET_LENGTH_STANDARD*7*400)\
	, /*ts_send_queue_(512), ts_send_unmlimt_queue_(1024*50),*/ play_stream_count_(0)/*, ts_task_pool_(4)*/\
	, io_svt_(std::make_shared<boost::asio::io_service>()), strand_(*io_svt_)\
	, stream_rate_(TESTReceiver::RateSD) 
{
	if (TESTReceiver::RateSD == stream_rate_)
	{
		ts_comm_send_queue_.ts_sd_send_queue_ = std::make_shared<TSSendSpscSDQueueType>();
	}
	else if (TESTReceiver::RateHD == stream_rate_)
	{
		ts_comm_send_queue_.ts_hd_send_queue_ = std::make_shared<TSSendSpscHDQueueType>();
	}
	else if (TESTReceiver::RateFHD == stream_rate_)
	{
		ts_comm_send_queue_.ts_fhd_send_queue_ = std::make_shared<TSSendSpscFHDQueueType>();
	}
	else if (TESTReceiver::Rate2K == stream_rate_)
	{
		ts_comm_send_queue_.ts_2k_send_queue_ = std::make_shared<TSSendSpsc2KQueueType>();
	}
	else if (TESTReceiver::Rate4K == stream_rate_)
	{
		ts_comm_send_queue_.ts_4k_send_queue_ = std::make_shared<TSSendSpsc4KQueueType>();
	}
	else
	{
		ts_comm_send_queue_.ts_fhd_send_queue_ = std::make_shared<TSSendSpscFHDQueueType>();
	}

	worker_ = std::make_shared<boost::asio::io_service::work>(*io_svt_);
	play_stream_ = uri_;
	ostringstream ss_out;
	{
		boost::uuids::random_generator sgen;
		boost::uuids::uuid uid = sgen();
		unique_str_ = boost::uuids::to_string(uid);
		std::string cur_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
		cur_date_ = boost::gregorian::to_iso_string(boost::gregorian::day_clock::local_day());
		ss_out << cur_time;
		ss_out << "-" << unique_str_ << ".xml";
		boost::filesystem::path ts_task_path = boost::filesystem::current_path();
		ts_task_path /= "task";
		ts_task_path /= cur_date_;
		if (!boost::filesystem::exists(ts_task_path))
		{
			boost::filesystem::create_directories(ts_task_path);
		}
		boost::filesystem::path whole_path = ts_task_path / ss_out.str();
		ts_task_file_name_ = whole_path.string();
		vvlog_i("cmd:" << play_stream_ << " ts tasks file name:" << ts_task_file_name_);
	}
}

TESTReceiver::~TESTReceiver()
{
	b_exit_ts_task_ = true;
	b_exit_parse_task_ = true;
	std::cout << "come desctruct Stream receiver" << std::endl;
	if (!ts_spsc_packet_queue.empty())
	{
		ts_spsc_packet_queue.consume_all([](TS_PACKET_CONTENT item) 
		{
			;
		});
	}
	//if (!ts_send_unmlimt_queue_.empty())
	//{
	//	ts_send_unmlimt_queue_.consume_all([](TSSENDCONTENT item)
	//	{
	//		;
	//	});
	//}
}

int TESTReceiver::start()
{
	if (nullptr != ts_thrd_ptr_)
	{
		vvlog_w("m3u8 and ts thread is started ts thread id:" << ts_thrd_ptr_->get_id());
		return E_OK;
	}

	ts_thrd_ptr_.reset(new std::thread(std::bind(&TESTReceiver::_do_ts_task, this)));
	if (ts_thrd_ptr_)
	{
		//m3u8_thrd_ptr_->detach();
		vvlog_i("start receive stream success url:" << uri_ << " pid:" << this_thread::get_id());
		return E_OK;
	}
	else
	{
		vvlog_e("start receive stream fail url:" << uri_ << " pid:" << this_thread::get_id());
		return E_PARAM_ERROR;
	}
}

void TESTReceiver::stop()
{
	b_exit_ts_task_ = true;
	if (ts_thrd_ptr_ && ts_thrd_ptr_->joinable())
	{
		ts_thrd_ptr_->join();
	}
	ts_thrd_ptr_ = nullptr;
	b_exit_parse_task_ = true;
	if (parse_ts_thrd_ && parse_ts_thrd_->joinable())
	{
		parse_ts_thrd_->join();
	} 
	parse_ts_thrd_ = nullptr;
	vvlog_i("stop receive stream url:" << uri_);
}


bool TESTReceiver::get_ts_send_sd_queue(TSSendSpscSDQueueTypePtr &ts_send_queue)
{
	if (ts_comm_send_queue_.ts_sd_send_queue_ != nullptr)
	{
		ts_send_queue = ts_comm_send_queue_.ts_sd_send_queue_;
		return true;
	}
	else
	{
		return false;
	}
}

bool TESTReceiver::get_ts_send_hd_queue(TSSendSpscHDQueueTypePtr &ts_send_queue)
{
	if (ts_comm_send_queue_.ts_hd_send_queue_ != nullptr)
	{
		ts_send_queue = ts_comm_send_queue_.ts_hd_send_queue_;
		return true;
	}
	else
	{
		return false;
	}

}

bool TESTReceiver::get_ts_send_fhd_queue(TSSendSpscFHDQueueTypePtr &ts_send_queue)
{
	if (ts_comm_send_queue_.ts_fhd_send_queue_ != nullptr)
	{
		ts_send_queue = ts_comm_send_queue_.ts_fhd_send_queue_;
		return true;
	}
	else
	{
		return false;
	}

}

bool TESTReceiver::get_ts_send_2k_queue(TSSendSpsc2KQueueTypePtr &ts_send_queue)
{
	if (ts_comm_send_queue_.ts_2k_send_queue_ != nullptr)
	{
		ts_send_queue = ts_comm_send_queue_.ts_2k_send_queue_;
		return true;
	}
	else
	{
		return false;
	}

}

bool TESTReceiver::get_ts_send_4k_queue(TSSendSpsc4KQueueTypePtr &ts_send_queue)
{
	if (ts_comm_send_queue_.ts_4k_send_queue_ != nullptr)
	{
		ts_send_queue = ts_comm_send_queue_.ts_4k_send_queue_;
		return true;
	}
	else
	{
		return false;
	}
}

int TESTReceiver::_push_ts_data_to_queue(string &ts_data)
{
	long int data_size = ts_data.length();
	TS_PACKET_CONTENT one_ts_data;
	long int data_index = 0;
	char *src_data = (char*)ts_data.data();
	if (data_size%TS_PACKET_LENGTH_STANDARD != 0)//数据不完整
	{
		assert(false);
	}
	while (TS_PACKET_LENGTH_STANDARD <= data_size)
	{
		memcpy(one_ts_data.content, src_data+data_index, TS_PACKET_LENGTH_STANDARD);
		one_ts_data.real_size = TS_PACKET_LENGTH_STANDARD;
		data_size -= TS_PACKET_LENGTH_STANDARD;
		data_index += TS_PACKET_LENGTH_STANDARD;
		while (!ts_spsc_packet_queue.push(one_ts_data))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		//memset(&one_ts_data, 0, sizeof(one_ts_data));
	}
	if (0 < data_size)
	{
		assert(false);
	}
	return data_size;
}

int TESTReceiver::tsCallback(char *data, const long int &data_len, const bool &is_finished)
{
	if (data && 0 < data_len)
	{
		//std::call_once(ts_data_flag_, [&]()
		//{
		//	ts_data_condition_.notify_all();
		//});
		if (!parse_ts_thrd_)
		{
			parse_ts_thrd_.reset(new std::thread(std::bind(&TESTReceiver::_do_parse_ts_data_ext, this)));
		}
		int push_result = receive_ts_buffer_.pushToBuffer(data, data_len);
		if (E_OK != push_result)
		{
			return push_result;
		}
		while (TS_PACKET_LENGTH_STANDARD*3 <= receive_ts_buffer_.data_len())
		{
			TS_PACKET_CONTENT one_ts_data;
			if (receive_ts_buffer_.pop_data(one_ts_data.content, TS_PACKET_LENGTH_STANDARD, TS_PACKET_LENGTH_STANDARD))
			{
				if (one_ts_data.content[0] != 0x47)
				{
					//assert(false);
					//vvlog_e("receive data error url:" << play_stream_ << "data:" << one_ts_data.content);
					memset(&one_ts_data, 0, sizeof(one_ts_data));
					continue;
				}
				one_ts_data.real_size = TS_PACKET_LENGTH_STANDARD;
				while(!ts_spsc_packet_queue.push(one_ts_data))
				{
					//std::this_thread::yield();
					std::this_thread::sleep_for(std::chrono::nanoseconds(1));
					if (b_exit_ts_task_)
					{
						break;
					}
					//vvlog_e("push ts data to queue faile, url!!");
				}
				memset(&one_ts_data, 0, sizeof(one_ts_data));
			}
			else
			{
				vvlog_e("pop data faile");
			}
		}
	}

#pragma region newhttp
	//ts_send_signal_(data, data_len, is_finished);
	return E_OK;
#pragma endregion newhttp
}

int TESTReceiver::_do_ts_task()
{
	vvlog_i("doing ts task cmd:" << play_stream_ << "pid:" << std::this_thread::get_id());
	while (1)
	{
		if (b_exit_ts_task_)
		{
			return E_OK;
		}
		FILE * ts_file_fd = fopen(play_stream_.c_str(), "rb");
		if (!ts_file_fd)
		{
			return E_FILE_ERROR;
		}
		char data_cache[188 * 200] = { 0 };
		while (!feof(ts_file_fd)) 
		{
			if (b_exit_ts_task_)
			{
				return E_OK;
			}
			int len = fread(data_cache, 1, 188 * 200, ts_file_fd);
			if (len > 0)
			{
				while (E_BUFFER_FULL == this->tsCallback(data_cache, len, false))
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			}
		}
		fclose(ts_file_fd);
	}
	return E_OK;
}

void TESTReceiver::_write_ts_file_list(const string &out_file_name, const string &ts_file_name, const int &index, const string &time)
{
	pugi::xml_node ts_file_node = ts_file_doc_.append_child("ts_file");
	pugi::xml_attribute name_attr = ts_file_node.append_attribute("name");
	name_attr.set_value(ts_file_name.c_str());
	pugi::xml_attribute index_attr = ts_file_node.append_attribute("index");
	index_attr.set_value(index);
	pugi::xml_attribute time_attr = ts_file_node.append_attribute("time");
	if (!time.empty())
	{
		time_attr.set_value(time.data());
	}
	std::string cur_date = boost::gregorian::to_iso_string(boost::gregorian::day_clock::local_day());
	if (cur_date != cur_date_)
	{
		ts_file_doc_.save_file(out_file_name.data(), "\t", pugi::encoding_utf8);
		ts_file_doc_.reset();

		cur_date_ = cur_date;
		boost::filesystem::path ts_task_path = boost::filesystem::current_path();
		ts_task_path /= "task";
		ts_task_path /= cur_date_;
		if (!boost::filesystem::exists(ts_task_path))
		{
			boost::filesystem::create_directories(ts_task_path);
		}
		ostringstream ss_out;
		std::string cur_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
		ss_out << cur_time;
		ss_out << "-" << unique_str_ << ".xml";
		boost::filesystem::path whole_path = ts_task_path / ss_out.str();
		ts_task_file_name_ = whole_path.string();
	}
}

void TESTReceiver::_write_content_to_file(const string &out_file_name, char *data, const int &data_len)
{
	FILE *open_file = fopen(out_file_name.c_str(), "ab");
	if (open_file)
	{
		//if (10 < data_len)
		//{
		//	if (*data == 0x00 && *(data + 1) == 0x00 && *(data + 3) == 0x00 && *(data + 5) == 0x00 && *(data + 9) == 0)
		//	{
		//		assert(false);
		//	}
		//}
		int write_size = fwrite(data, sizeof(char), data_len, open_file);
		if (0 > write_size)
		{
#ifdef _WIN32
			//vvlog_e("write file:" << out_file_name << "error:" << GetLastError());
#else
			//vvlog_e("write file:" << out_file_name << "error:" << errno);
#endif // _WIN32
			//assert(false);
		}
		else if (write_size != data_len)
		{
#ifdef _WIN32
			//vvlog_e("write file:" << out_file_name << "error:" << GetLastError());
#else
			//vvlog_e("write file:" << out_file_name << "error:" << errno);
#endif
		}
		fclose(open_file);
	}
	else
	{
#ifdef _WIN32
		std::cout << out_file_name <<" file open error:" << GetLastError() << std::endl;
#else
		//vvlog_e("open file:" << out_file_name << "error:" << errno);
		perror("file open error");
#endif // _DEBUG
	}
	return;
	fstream out_file;
	//char out_file_name[100] = {0};
	//sprintf(out_file_name, "out_file_%d.ts", save_content_index_);
	//save_content_index_++;
	out_file.open(out_file_name.data(), ios::out | ios::binary | ios::app);
	if (out_file.is_open())
	{
		out_file.write(data, data_len);
		out_file.flush();
		out_file.close();
	}
	else
	{
		std::cout << "open file:" << out_file_name << " faile!!!";
	}
}

void TESTReceiver::_do_parse_ts_data()
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
	std::cout << "parse ts task:" << std::this_thread::get_id() <<std::endl;
	while (1)
	{
		if (b_exit_parse_task_)
		{
			break;
		}
		while (ts_spsc_packet_queue.pop(ts_data))
		{
			if (ts_data.real_size < TS_PACKET_LENGTH_STANDARD)
			{
				vvlog_e("ts not complete packet");
			}
			if (!ts_data.content || ts_data.content[0] != 0x47)
			{
				assert(false);
			}
			if (ts_packet.SetPacket((BYTE*)ts_data.content))
			{
				PCR packet_pcr = ts_packet.Get_PCR();
				int pid = 0;
				get_pcr((BYTE*)ts_data.content, &packet_pcr, &pid);
				//WORD pid = ts_packet.Get_PID();
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
						//std::cout << "num:" << pcr_duration_packet_num << std::endl;
						long int system_clock_reference = 27;
						double tranlate_rate = (double)(8 * pcr_duration_packet_num*TS_PACKET_LENGTH_STANDARD*system_clock_reference) / (pcr_end - pcr_front);
						double tranlate_interval_time = (double)(TS_PACKET_LENGTH_STANDARD * 7 * 8*1000) / (long)(tranlate_rate*1000);//发送188*7需要的时间 micro
						//vvlog_i("url:" << play_stream_ << "tsNum:" << pcr_duration_packet_num << "first_pcr:" << pcr_front << "pcr_end:" << pcr_end\
							<< "rate:" << tranlate_rate_byte_micro << "time:" << tranlate_interval_time);
						pcr_first_packet_num = pcr_second_packet_num = ts_packet_num = 0;
						_push_ts_data_to_send_queue(sender_buffer.get_data(), sender_buffer.get_data_length(), tranlate_interval_time);
						sender_buffer.reset_buffer();
					}
				}
			}
			else
			{
				//if (out_ts_file_)
				//{
				//	fwrite(ts_data.content, 1, ts_data.real_size, out_ts_file_);
				//}
				std::cout << "invaild packet, url:" << std::endl;
				//vvlog_e("it is a invalid packet!! receiverurl:"<<receive_url_ << "len:" << ts_data.real_size);
			}
			memset(&ts_data, 0, sizeof(ts_data));
		}
		//memset(&ts_data, 0, sizeof(ts_data));
		std::this_thread::yield();
		//this_thread::sleep_for(std::chrono::microseconds(1));
	}

}

void TESTReceiver::_do_parse_ts_data_ext()
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
	vvlog_i("doing parse ts task cmd:" << play_stream_ << "pid:" << std::this_thread::get_id());
	int push_count = 0;
	while (1)
	{
		if (b_exit_parse_task_)
		{
			break;
		}
		while (ts_spsc_packet_queue.pop(ts_data))
		{
			if (ts_data.real_size < TS_PACKET_LENGTH_STANDARD)
			{
				vvlog_e("ts not complete packet, url:"<< play_stream_ << "size:" << ts_data.real_size);
			}
			if (0 < ts_data.real_size && ts_data.content[0] != 0x47)
			{
				assert(false);
			}
			if (ts_packet.SetPacket((BYTE*)ts_data.content))
			{
				PCR packet_pcr = ts_packet.Get_PCR();
				//WORD pid = ts_packet.Get_PID();
				int pid = 0;
				get_pcr((BYTE*)ts_data.content, &packet_pcr, &pid);
				int result = sender_buffer.push_to_buffer(ts_data.content, ts_data.real_size);
				ts_packet_num++;
				if (INVALID_PCR != packet_pcr ||ts_packet_num == 7)
				{
					TSSENDCONTENT ts_send_content;
					memcpy(ts_send_content.content, sender_buffer.get_data(), sender_buffer.get_data_length());
					ts_send_content.real_size = sender_buffer.get_data_length();
					if (packet_pcr != INVALID_PCR)
					{
#if ENABLE_OUT_PCR
						std::cout << "pcr:" << packet_pcr << std::endl;
#endif
						ts_send_content.cur_pcr = packet_pcr;
						ts_send_content.is_real_pcr = true;
					}
					//std::cout << "push ts" << std::endl;
					if (ts_comm_send_queue_.ts_sd_send_queue_)
					{
						_doing_push_ts_data_to_queue(ts_comm_send_queue_.ts_sd_send_queue_, ts_send_content);
					}
					else if (ts_comm_send_queue_.ts_hd_send_queue_)
					{
						_doing_push_ts_data_to_queue(ts_comm_send_queue_.ts_hd_send_queue_, ts_send_content);
					}
					else if (ts_comm_send_queue_.ts_fhd_send_queue_)
					{
						_doing_push_ts_data_to_queue(ts_comm_send_queue_.ts_fhd_send_queue_, ts_send_content);
					}
					else if (ts_comm_send_queue_.ts_2k_send_queue_)
					{
						_doing_push_ts_data_to_queue(ts_comm_send_queue_.ts_2k_send_queue_, ts_send_content);
					}
					else if (ts_comm_send_queue_.ts_4k_send_queue_)
					{
						_doing_push_ts_data_to_queue(ts_comm_send_queue_.ts_4k_send_queue_, ts_send_content);
					}
					//while (1 > ts_send_queue_.write_available())
					//{
					//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
					//}
					//if (!ts_send_queue_.push(ts_send_content))
					//{
					//	std::cout << "push ts data faile" << std::endl;
					//}
					//ts_send_unmlimt_queue_.push(ts_send_content);
					ts_packet_num = 0;
					sender_buffer.reset_buffer();
				}
			}
			else
			{
				//if (out_ts_file_)
				//{
				//	fwrite(ts_data.content, 1, ts_data.real_size, out_ts_file_);
				//}
				std::cout << "invaild packet, url:" << std::endl;
				//vvlog_e("it is a invalid packet!! receiverurl:"<<receive_url_ << "len:" << ts_data.real_size);
			}
			memset(&ts_data, 0, sizeof(ts_data));
		}
		//this_thread::yield();
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}


}

void TESTReceiver::_push_ts_data_to_send_queue(char *data, const long int &data_len, const int &need_time/*bytes:(188*7):micro*/)
{
	char *tmp_data = data;
	long int tmp_data_len = data_len;
	TS_SEND_CONTENT ts_send_content;
	while (TS_SEND_SIZE <= tmp_data_len)
	{
		memcpy(ts_send_content.content, tmp_data, TS_SEND_SIZE);
		//ts_send_content.need_time = need_time;
		ts_send_content.real_size = TS_SEND_SIZE;
		if (tmp_data_len == TS_SEND_SIZE)
		{
			ts_send_content.is_real_pcr = true;
		}
		try {
			//if (!ts_send_unmlimt_queue_.bounded_push(ts_send_content))
			//if (!ts_send_unmlimt_queue_.push(ts_send_content))
			//{
			//	std::cout << "push send content fail" << std::endl;
			//}
		}
		catch (std::exception e)
		{
			std::cout << "errmsg:" << e.what() << std::endl;
		}
		//while (!ts_send_spsc_queue_.push(ts_send_content))
		//{
		//	if (b_exit_parse_task_)
		//	{
		//		break;
		//	}
		//	this_thread::yield();
		//	//this_thread::sleep_for(std::chrono::microseconds(1));
		//	//vvlog_e("push ts send content to queue faile, url:"<< play_stream_);
		//}
		tmp_data_len -=TS_SEND_SIZE;
		tmp_data = tmp_data + TS_SEND_SIZE;
		memset(&ts_send_content, 0, sizeof(ts_send_content));
	}
	if (0 < tmp_data_len)
	{
		memcpy(ts_send_content.content, tmp_data, tmp_data_len);
		//ts_send_content.need_time = (tmp_data_len*need_time) / (TS_SEND_SIZE);
		ts_send_content.real_size = tmp_data_len;
		ts_send_content.is_real_pcr = true;
		try
		{
			//if (!ts_send_unmlimt_queue_.bounded_push(ts_send_content))
			//if (!ts_send_unmlimt_queue_.push(ts_send_content))
			//{
			//	std::cout << "push send content fail" << std::endl;
			//}
		}
		catch (std::exception e)
		{
			std::cout << "errmsg:" << e.what() << std::endl;
		}
		//while (!ts_send_spsc_queue_.push(ts_send_content))
		//{
		//	if (b_exit_parse_task_)
		//	{
		//		break;
		//	}
		//	std::this_thread::yield();
		//	//std::this_thread::sleep_for(std::chrono::microseconds(1));
		//	//vvlog_e("push ts send content to queue fail, url:" << play_stream_);
		//}
		memset(&ts_send_content, 0, sizeof(ts_send_content));
	}
}


int TESTReceiver::_do_ts_task_group()
{
	while (1)
	{
		HTTPTSCMD ts_cmd;
		if (ts_task_list_.pop(ts_cmd))
		{

		}
	}
}

