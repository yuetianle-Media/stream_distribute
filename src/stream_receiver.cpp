#include "stdafx.h"
#include "stream_receiver.h"



StreamReceiver::StreamReceiver(const string &url)
	:ts_task_list_(5000), uri_parser_(url), play_stream_duration_(0)\
	, m3u8_thrd_ptr_(nullptr), ts_thrd_ptr_(nullptr), parse_ts_thrd_(nullptr)\
	, b_exit_m3u8_task_(false), b_exit_ts_task_(false), b_exit_parse_task_(false)\
	, ts_file_index_(0), uri_(url), receive_ts_buffer_(TS_PACKET_LENGTH_STANDARD*7*400)\
	, ts_send_unmlimt_queue_(1024*50), play_stream_count_(0)/*, ts_task_pool_(4)*/\
	, io_svt_(std::make_shared<boost::asio::io_service>()), strand_(*io_svt_)
{
	worker_ = std::make_shared<boost::asio::io_service::work>(*io_svt_);
	play_stream_ = uri_;
	ostringstream ss_out;
	if (uri_parser_.is_ready())
	{
		ss_out << uri_parser_.host() << "-" << uri_parser_.port() << "-";
		ss_out << boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()) << ".xml";
		ts_task_file_name_ = ss_out.str();
	}
	if (!uri_.empty())
	{
		m3u8_http_client_ptr_ = std::make_shared<HttpCurlClient>();
		ts_http_client_ptr_   = std::make_shared<HttpCurlClient>();
	}
}

StreamReceiver::~StreamReceiver()
{
	b_exit_m3u8_task_ = true;
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
	if (!ts_send_unmlimt_queue_.empty())
	{
		ts_send_unmlimt_queue_.consume_all([](TSSENDCONTENT item)
		{
			;
		});
	}
}

int StreamReceiver::start()
{
	if (nullptr != m3u8_thrd_ptr_ && nullptr != ts_thrd_ptr_)
	{
		vvlog_w("m3u8 and ts thread is started m3u8 thread id:" << m3u8_thrd_ptr_->get_id()\
			<< "ts thread id:" << ts_thrd_ptr_->get_id());
		return E_OK;
	}

	if (nullptr != m3u8_http_client_ptr_)
	{
		m3u8_conn_ = m3u8_http_client_ptr_->subscribe_data(boost::bind(&StreamReceiver::m3u8Callback, this, _1, _2, _3));
	}
	if (nullptr != ts_http_client_ptr_)
	{
		ts_conn_ = ts_http_client_ptr_->subscribe_data(boost::bind(&StreamReceiver::tsCallback, this, _1, _2, _3));
	}
	//m3u8_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_m3u8_task, this)));
	m3u8_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_m3u8_task_boost, this)));
	//ts_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_ts_task, this)));
	ts_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_ts_task_boost, this)));
	//parse_ts_thrd_.reset(new thread(std::bind(&StreamReceiver::_do_parse_ts_data, this)));
	parse_ts_thrd_.reset(new std::thread(std::bind(&StreamReceiver::_do_parse_ts_data_ext, this)));
	if (m3u8_thrd_ptr_ && ts_thrd_ptr_ && parse_ts_thrd_)
	{
		//m3u8_thrd_ptr_->detach();
		//ts_thrd_ptr_->detach();
		//parse_ts_thrd_->detach();
		int concurrency = m3u8_thrd_ptr_->hardware_concurrency();
		vvlog_i("start receive stream success url:" << uri_);
		return E_OK;
	}
	else
	{
		vvlog_e("start receive stream fail url:" << uri_);
		return E_PARAM_ERROR;
	}
}

void StreamReceiver::stop()
{
	b_exit_m3u8_task_ = true;
	if (m3u8_thrd_ptr_ && m3u8_thrd_ptr_->joinable())
	{
		m3u8_thrd_ptr_->join();
	}
	m3u8_thrd_ptr_ = nullptr;
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
	if (m3u8_http_client_ptr_)
	{
		m3u8_http_client_ptr_->unsubscribe_data();
	}
	if (ts_http_client_ptr_)
	{
		ts_http_client_ptr_->unsubscribe_data();
	}
	vvlog_i("stop receive stream url:" << uri_);
}


boost::signals2::connection StreamReceiver::subcribe_ts_callback(const TsSignal::slot_type &slot)
{
	return ts_send_signal_.connect(slot);
}

std::string StreamReceiver::_make_m3u8_cmd(const string &stream_url/*=0*/)
{
	std::stringstream ss;
	ss << "GET " << "/" << uri_parser_.resource_path() << " HTTP/1.1" << "\r\n"\
		<< "Host: " << uri_parser_.host() << ":" << uri_parser_.port() << " \r\n"\
		<< "User-Agent: " << "me-test" << "\r\n"\
		//<< "Accept: " << "test/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" << "\r\n"
		<< "Accept: " << "test/html,application/xhtml+xml,application/xml" << "\r\n"\
		<< "Connection: " << "keep-alive" << "\r\n\r\n";
	return ss.str();
}

bool StreamReceiver::_make_m3u8_cmd(HTTPM3U8CMD & m3u8_cmd, const std::string & stream_url)
{
	std::stringstream ss;
	if (stream_url.empty())
	{
		if (!uri_parser_.is_ready())
		{
			return false;
		}
		ss << "GET " << "/" << uri_parser_.resource_path() << " HTTP/1.1" << "\r\n"\
			<< "Host: " << uri_parser_.host() << ":" << uri_parser_.port() << " \r\n"\
			<< "User-Agent: " << "me-test" << "\r\n"\
			//<< "Accept: " << "test/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" << "\r\n"
			<< "Accept: " << "test/html,application/xhtml+xml,application/xml" << "\r\n"\
			<< "Connection: " << "keep-alive" << "\r\n\r\n";
#ifdef _WIN32
		memcpy_s(m3u8_cmd.cmd, sizeof(m3u8_cmd.cmd), ss.str().c_str(), ss.str().length());
#else
		memcpy(m3u8_cmd.cmd, ss.str().c_str(), ss.str().length());
#endif
		m3u8_cmd.cmd_length = ss.str().length();
		return true;
	}
	else
	{
		URIParser uri_parser(stream_url);
		if (!uri_parser.is_ready())
		{
			return false;
		}
		ss << "GET " << "/" << uri_parser.resource_path() << " HTTP/1.1" << "\r\n"\
			<< "Host: " << uri_parser.host() << ":" << uri_parser.port() << " \r\n"\
			<< "User-Agent: " << "me-test" << "\r\n"\
			<< "Accept: " << "test/html,application/xhtml+xml,application/xml" << "\r\n"\
			<< "Connection: " << "keep-alive" << "\r\n\r\n";
#ifdef _WIN32
		memcpy_s(m3u8_cmd.cmd, sizeof(m3u8_cmd.cmd), ss.str().c_str(), ss.str().length());
#else
		memcpy(m3u8_cmd.cmd, ss.str().c_str(), ss.str().length());
#endif
		m3u8_cmd.cmd_length = ss.str().length();
		return true;
	}
}

std::string StreamReceiver::_make_down_ts_cmd(const string &ts_file)
{
	std::stringstream ss;
	boost::filesystem::path ts_path(uri_parser_.uri());
	ts_path.remove_filename();
	std::string ts_uri = ts_path.string() + "/" + ts_file;
	/*URIParser ts_uri_parser(ts_uri);
	ss << "GET " << "/" << ts_uri_parser.resource_path() << " HTTP/1.1" << "\r\n"\
		<< "Host: " << ts_uri_parser.host() << ":" << ts_uri_parser.port() << " \r\n"\
		<< "User-Agent: " << "me-test" << "\r\n"\
		<< "Accept: " << "test/html,application/xhtml+xml,application/xml" << "\r\n"\
		<< "Connection: " << "keep-alive" << "\r\n\r\n";
		*/
	return ts_uri;
}

bool StreamReceiver::_make_down_ts_cmd(HTTPTSCMD &ts_cmd, const std::string &ts_file)
{
	return true;
}

int StreamReceiver::_send_m3u8_cmd(const string &m3u8_cmd)
{
	int result = E_OK;
	if (m3u8_http_client_ptr_)
	{
		//vvlog_i("send m3u8 cmd:" << m3u8_cmd);
		result = m3u8_http_client_ptr_->get(m3u8_cmd);
		if (E_OK != result)
		{
			//vvlog_e("send m3u8 cmd" << m3u8_cmd << " fail!");
			//std::cout << "send m3u8 cmd" << m3u8_cmd << " fail!" << std::endl;
		}
		else
		{
			//vvlog_i("send m3u8 cmd:" << m3u8_cmd << "success");
			//std::cout << "send ts cmd:" << m3u8_cmd << "success" << std::endl;
		}
	}
	return E_OK;
}

int StreamReceiver::_send_ts_cmd(const string &ts_cmd)
{
	int result = E_OK;
    if (nullptr != ts_http_client_ptr_)
    {
		//vvlog_i("send ts cmd:" << ts_cmd << "start pid:" << std::this_thread::get_id());
		//auto ret = std::async(std::launch::async, [&]() {

		//	result = ts_http_client_ptr_->get(ts_cmd);
		//	return result;
		//});
		//result = ret.get();
		result = ts_http_client_ptr_->get(ts_cmd);
		if (E_OK != result)
		{
			vvlog_e("send ts cmd : " << ts_cmd << " fail!");
			//std::cout << "send ts cmd:" << ts_cmd << " fail!" << std::endl;
		}
		else
		{
			//vvlog_i("send ts cmd:" << ts_cmd << "success");
			//std::cout << "send ts cmd:" << ts_cmd << "success" << std::endl;
		}
    }
	return result;
}

int StreamReceiver::_send_ts_cmd_boost(const std::string &ts_cmd)
{
	int result = E_OK;
	try
	{
		boost::network::http::client::request ts_request;
		boost::network::http::client::response ts_response;
		ts_request.uri(ts_cmd);
		ts_response = ts_boost_http_client_.get(ts_request);
		//boost::shared_future<uint16_t> status_code;
		//ts_response.status(status_code);
		//if (200 ==  status_code.get())
		if (200 == ts_response.status())
		{
			//boost::shared_future<std::string> ts_data_out;
			//ts_response.body(ts_data_out);
			std::string ts_data = ts_response.body();
			_push_ts_data_to_queue(ts_data);
		}
		else
		{
			return E_DATA_EMPTY;
		}
	}
	catch (std::exception e)
	{
		std::cout << "error:" << e.what() << std::endl;
	}
	return result;

}

int StreamReceiver::_push_ts_data_to_queue(string &ts_data)
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
	}
	if (0 < data_size)
	{
		assert(false);
	}
	return E_OK;
}

int StreamReceiver::_push_ts_cmd_to_queue(const string &ts_cmd_str)
{
	HTTPTSCMD ts_cmd;
	memcpy(ts_cmd.cmd, ts_cmd_str.c_str(), ts_cmd_str.length());
	ts_cmd.cmd_length = ts_cmd_str.length();
	std::map<string, string>::iterator iter = ts_all_task_map_.find(ts_cmd_str);
	if (iter == ts_all_task_map_.end())//此任务没有添加过
	{
		//vvlog_i("push ts task cout:" << ts_all_task_map_.size() << "task:" << ts_cmd_str);
		boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
		std::string cur_time = boost::posix_time::to_iso_string(now);
		ts_all_task_map_.insert(std::make_pair(ts_cmd_str, cur_time));
		if (!ts_task_list_.push(ts_cmd))
		{
			vvlog_e("push ts task faile tscmd:" << ts_cmd_str);
		}
		if (!ts_task_file_name_.empty())
		{
			_write_ts_file_list(ts_task_file_name_, ts_cmd_str, 0);
		}
	}
	return 0;
}

void StreamReceiver::m3u8Callback(char *data, const long int &data_len, const bool &is_finished)
{
#pragma region new
	if (data && 0 < data_len)
	{
		m3u8_content_.append(data, data_len);
	}
	if (is_finished)
	{
		//m3u8_content_.append(data, data_len);
		M3u8Parser parser(play_stream_, m3u8_content_);
		if (parser.is_variant_play_list())//多个视频流
		{
			PlayListList* play_lists = nullptr;
			if (parser.get_play_list(play_lists) && play_lists)
			{
				//int play_stream_index = 0;
				for (auto play_list : *play_lists)
				{
					bool is_same = false;
					for (auto item : play_stream_list_)
					{
						if (item.second.compare(play_list.uri) == 0)//播放列表中已经存在
						{
							is_same = true;
							break;
						}
					}
					if (!is_same)
					{
						play_stream_list_.insert(std::make_pair(play_stream_count_, play_list.uri));
						play_stream_count_++;
					}
				}
			}
		}
		else
		{
			play_stream_duration_ = (int)parser.get_max_duration();
			M3U8SegmentList *segment_list = nullptr;
			if (parser.get_segments(segment_list) && segment_list)
			{
				for (auto segment : *segment_list)
				{
					_push_ts_cmd_to_queue(segment.uri);
					//将当前2分钟之前的task从map中删除掉
					_remove_ts_cmd(M3U8TASKREMOVEINTERVAL);
				}
			}
		}
		m3u8_content_.clear();
	}
#pragma region test
	//fstream out_file;
	//char file_name[100] = {0};
	//sprintf(file_name, "m3u8_%d.txt", ts_file_index_++);
	//out_file.open(file_name, ios::out | ios::binary | ios::app);
	//out_file.write(data, data_len);
	//out_file.close();
    //解析mu38文件, 生成ts下载命令插入任务队列
#pragma endregion test
}

void StreamReceiver::tsCallback(char *data, const long int &data_len, const bool &is_finished)
{
	if (data && 0 < data_len)
	{
		int push_result = receive_ts_buffer_.pushToBuffer(data, data_len);
		if (E_OK != push_result)
		{
			assert(false);
		}
		while (TS_PACKET_LENGTH_STANDARD <= receive_ts_buffer_.data_len())
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
	return;
#pragma endregion newhttp
}

int StreamReceiver::_do_m3u8_task()
{
	std::cout << "m3u8Task pid:" << std::this_thread::get_id() << std::endl;
	boost::asio::steady_timer wait_timer(*io_svt_);
    while (1) {
        if (b_exit_m3u8_task_)
        {
            break;
        }
		if (0 < play_stream_.length())
		{
			if (0 < play_stream_duration_)
			{
				wait_timer.expires_from_now(std::chrono::seconds(play_stream_duration_));
			}
			//vvlog_i("send m3u8 start cmd:" << play_stream_);
            int result = _send_m3u8_cmd(play_stream_);
			if (result != CURLE_OK)//请求失败或数据不正确
			{
				std::this_thread::sleep_for(std::chrono::seconds(5));
				wait_timer.cancel();
			}
			else
			{
				wait_timer.wait();
			}
			//vvlog_i("send m3u8 end cmd:" << play_stream_);
			if (0 < play_stream_count_ && 0 < play_stream_list_.size())
			{
				play_stream_ = play_stream_list_.at(play_stream_count_-1);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}
    }
	return 0;
}

int StreamReceiver::_do_m3u8_task_boost()
{
	std::cout << "m3u8Task pid:" << std::this_thread::get_id() << std::endl;
	boost::asio::steady_timer wait_timer(*io_svt_);
	boost::network::http::client::request m3u8_request;
	boost::network::http::client::response m3u8_response;
	while (1)
	{
		if (b_exit_m3u8_task_)
		{
			break;
		}
		if (0 < play_stream_.length())
		{
			m3u8_request.uri(play_stream_);
			if (0 < play_stream_duration_)
			{
				wait_timer.expires_from_now(std::chrono::seconds(play_stream_duration_));
			}
			bool is_success = false;
			try
			{
				m3u8_response = m3u8_boost_http_client_.get(m3u8_request);
				m3u8_content_ = m3u8_response.body();
				if (m3u8_response.status() == 200)
					is_success = true;
			}
			catch (std::exception e)
			{
				std::cout << e.what() << std::endl;
			}
			if (is_success)
			{
				M3u8Parser parser(play_stream_, m3u8_content_);
				if (parser.is_variant_play_list())//多个视频流
				{
					PlayListList* play_lists = nullptr;
					if (parser.get_play_list(play_lists) && play_lists)
					{
						//int play_stream_index = 0;
						for (auto play_list : *play_lists)
						{
							bool is_same = false;
							for (auto item : play_stream_list_)
							{
								if (item.second.compare(play_list.uri) == 0)//播放列表中已经存在
								{
									is_same = true;
									break;
								}
							}
							if (!is_same)
							{
								play_stream_list_.insert(std::make_pair(play_stream_count_, play_list.uri));
								play_stream_count_++;
							}
						}
					}
				}
				else
				{
					play_stream_duration_ = (int)parser.get_max_duration();
					//play_stream_duration_ = 0;
					M3U8SegmentList *segment_list = nullptr;
					if (parser.get_segments(segment_list) && segment_list)
					{
						for (auto segment : *segment_list)
						{
							//if (play_stream_duration_ < segment.duration)
							//{
							//	play_stream_duration_ = segment.duration;
							//}
							_push_ts_cmd_to_queue(segment.uri);
							//将当前2分钟之前的task从map中删除掉
							_remove_ts_cmd(M3U8TASKREMOVEINTERVAL);
						}
					}
				}
				m3u8_content_.clear();
			}
			if (0 > play_stream_duration_)
			{
				std::this_thread::sleep_for(std::chrono::seconds(5));
			}
			else
			{
				wait_timer.wait();
			}
			if (0 < play_stream_count_ && 0 < play_stream_list_.size())
			{
				play_stream_ = play_stream_list_.at(play_stream_count_-1);
			}
		}
	}
	return 0;
}

int StreamReceiver::_do_m3u8_task_group(const string & play_stream, const long int play_duration)
{
	auto self = shared_from_this();
	if (0 < play_stream_list_.size())
	{
		for (auto play_stream : play_stream_list_)
		{
			auto task_thread = std::make_shared<std::thread>(std::bind([&]() 
			{
				int play_duration = 0;
				std::string m3u8_content;
				auto http_client = std::make_shared<HttpCurlClient>();
				//TSTaskType ts_task_queue(1000);
				std::shared_ptr<TSTaskType> ts_queue_ptr = std::make_shared<TSTaskType>(1000);
				ts_task_group_.insert(std::make_pair(play_stream.first, ts_queue_ptr));
				std::map<std::string, std::string> all_ts_task;
				http_client->subscribe_data([&](char* data, const long &data_len, const bool &is_finished)->void 
				{
					m3u8_content.append(data, data_len);
					if (is_finished)
					{
						m3u8_content.append(data, data_len);
						M3u8Parser parser(play_stream.second, m3u8_content);
						if (!parser.is_variant_play_list())//单个视频流
						{
							play_duration = parser.get_max_duration();
							M3U8SegmentList *segment_list = nullptr;
							if (parser.get_segments(segment_list) && segment_list)
							{
								for (auto segment : *segment_list)
								{
									std::string ts_cmd_str = segment.uri;
									HTTPTSCMD ts_cmd;
									memcpy(ts_cmd.cmd, ts_cmd_str.data(), segment.uri.length());
									ts_cmd.cmd_length = ts_cmd_str.length();
									std::map<string, string>::iterator iter = all_ts_task.find(ts_cmd_str);
									if (iter == all_ts_task.end())//此任务没有添加过
									{
										boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
										std::string cur_time = boost::posix_time::to_iso_string(now);
										all_ts_task.insert(std::make_pair(ts_cmd_str, cur_time));
										ts_queue_ptr->push(ts_cmd);
									}
									//将当前10分钟之前的task从map中删除掉
								}
							}
						}
						m3u8_content.clear();
					}
				});
				while (1)
				{
					http_client->get(play_stream.second);
					std::this_thread::sleep_for(std::chrono::seconds(play_duration));
				}
			}));
		}
	}
	return 0;
}

int StreamReceiver::_do_ts_task()
{
	std::cout << "one ts task pid:" << std::this_thread::get_id() << std::endl;
    while (1) 
	{
		if (b_exit_ts_task_)
		{
			break;
		}
		ts_task_list_.consume_all([&](HTTPTSCMD ts_cmd)
		{
			if (b_exit_ts_task_)
			{
				return;
			}
			vvlog_i("send ts start cmd:" << ts_cmd.cmd);
			_send_ts_cmd(ts_cmd.cmd);
			vvlog_i("send ts end cmd:" << ts_cmd.cmd);
		});
		//HTTPTSCMD ts_cmd_struct;
		//while (ts_task_list_.pop(ts_cmd_struct))
		//{
		//	if (b_exit_ts_task_)
		//	{
		//		break;
		//	}
		//	//vvlog_i("send ts start url:" << ts_cmd_struct.cmd << "no task count:" << no_task_time_count);
		//	//auto result = ts_task_pool_.enqueue(std::bind(&StreamReceiver::_send_ts_cmd, this, ts_cmd_struct.cmd));
		//	//result.get();
		//	//vvlog_i("send ts end url:" << ts_cmd_struct.cmd);
		//	_send_ts_cmd(ts_cmd_struct.cmd);
		//}
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
	return E_OK;
}

int StreamReceiver::_do_ts_task_boost()
{
	std::cout << "one ts task pid:" << std::this_thread::get_id() << std::endl;
    while (1) 
	{
		if (b_exit_ts_task_)
		{
			break;
		}
		ts_task_list_.consume_all([&](HTTPTSCMD ts_cmd)
		{
			if (b_exit_ts_task_)
			{
				return;
			}
			vvlog_i("send ts start cmd:" << ts_cmd.cmd);
			_send_ts_cmd_boost(ts_cmd.cmd);
			vvlog_i("send ts end cmd:" << ts_cmd.cmd);
		});
		//HTTPTSCMD ts_cmd_struct;
		//while (ts_task_list_.pop(ts_cmd_struct))
		//{
		//	if (b_exit_ts_task_)
		//	{
		//		break;
		//	}
		//	//vvlog_i("send ts start url:" << ts_cmd_struct.cmd << "no task count:" << no_task_time_count);
		//	//auto result = ts_task_pool_.enqueue(std::bind(&StreamReceiver::_send_ts_cmd, this, ts_cmd_struct.cmd));
		//	//result.get();
		//	//vvlog_i("send ts end url:" << ts_cmd_struct.cmd);
		//	_send_ts_cmd(ts_cmd_struct.cmd);
		//}
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
	return E_OK;

}

void StreamReceiver::_write_ts_file_list(const string &out_file_name, const string &ts_file_name, const int &index, const string &time)
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
	ts_file_doc_.save_file(out_file_name.data(), "\t", pugi::encoding_utf8);
}

void StreamReceiver::_write_content_to_file(const string &out_file_name, char *data, const int &data_len)
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

void StreamReceiver::unsubcribe_ts_callback()
{
	ts_send_signal_.disconnect_all_slots();
}

void StreamReceiver::_do_parse_ts_data()
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

void StreamReceiver::_do_parse_ts_data_ext()
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
				vvlog_e("ts not complete packet");
			}
			if (!ts_data.content || ts_data.content[0] != 0x47)
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
					ts_send_unmlimt_queue_.push(ts_send_content);
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
		}
		//this_thread::yield();
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}


}

void StreamReceiver::_push_ts_data_to_send_queue(char *data, const long int &data_len, const int &need_time/*bytes:(188*7):micro*/)
{
	char *tmp_data = data;
	long int tmp_data_len = data_len;
	TS_SEND_CONTENT ts_send_content;
	while (TS_SEND_SIZE <= tmp_data_len)
	{
		memcpy(ts_send_content.content, tmp_data, TS_SEND_SIZE);
		ts_send_content.need_time = need_time;
		ts_send_content.real_size = TS_SEND_SIZE;
		if (tmp_data_len == TS_SEND_SIZE)
		{
			ts_send_content.is_real_pcr = true;
		}
		try {
			//if (!ts_send_unmlimt_queue_.bounded_push(ts_send_content))
			if (!ts_send_unmlimt_queue_.push(ts_send_content))
			{
				std::cout << "push send content fail" << std::endl;
			}
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
		ts_send_content.need_time = (tmp_data_len*need_time) / (TS_SEND_SIZE);
		ts_send_content.real_size = tmp_data_len;
		ts_send_content.is_real_pcr = true;
		try
		{
			//if (!ts_send_unmlimt_queue_.bounded_push(ts_send_content))
			if (!ts_send_unmlimt_queue_.push(ts_send_content))
			{
				std::cout << "push send content fail" << std::endl;
			}
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

int StreamReceiver::_remove_ts_cmd(const long int time_second)
{
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	boost::posix_time::ptime del_time = now - boost::posix_time::seconds(time_second);
	std::vector<std::string> remove_task;
	for (auto item : ts_all_task_map_)
	{
		boost::posix_time::ptime task_time(boost::posix_time::from_iso_string(item.second));
		if (task_time < del_time)
		{
			remove_task.push_back(item.first);
		}
	}
	for (auto del_task : remove_task)
	{
		ts_all_task_map_.erase(del_task);
	}
	return E_OK;
}

