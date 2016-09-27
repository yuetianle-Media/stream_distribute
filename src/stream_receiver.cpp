#include "stdafx.h"
#include "stream_receiver.h"


StreamReceiver::StreamReceiver(const string &url)
	:ts_task_list_(10000), uri_parser_(url), play_stream_duration_(5)\
	, m3u8_thrd_ptr_(nullptr), ts_thrd_ptr_(nullptr), b_exit(false)\
	, ts_file_index_(0), save_content_index_(0)
{
	play_stream_ = _make_m3u8_cmd(url);
	m3u8_tcp_client_ptr_	= std::make_shared<TCPClient>(uri_parser_.host(), uri_parser_.port(), 5000);
	ts_tcp_client_ptr_		= std::make_shared<TCPClient>(uri_parser_.host(), uri_parser_.port(), 5000);

}

StreamReceiver::~StreamReceiver()
{
	std::cout << "come desctruct Stream receiver" << std::endl;
}

int StreamReceiver::start()
{
	if (nullptr != m3u8_thrd_ptr_ && nullptr != ts_thrd_ptr_)
	{
		std::cout << "m3u8 and ts thread is started m3u8 thread id:" << m3u8_thrd_ptr_->get_id()\
			<< "ts thread id:" << ts_thrd_ptr_->get_id() << endl;
		return E_OK;
	}
    if (nullptr != m3u8_tcp_client_ptr_)
    {
        m3u8_conn =  m3u8_tcp_client_ptr_->subcribe_data_callback(boost::bind(&StreamReceiver::m3u8Callback, this, _1, _2));
		//m3u8_tcp_client_ptr_->send(play_stream_.data(), play_stream_.length());
		m3u8_tcp_client_ptr_->connect();
		m3u8_tcp_client_ptr_->async_send(play_stream_.data(), play_stream_.length());
		m3u8_tcp_client_ptr_->wait_response();
    }
    if (nullptr != ts_tcp_client_ptr_)
    {
        ts_conn = ts_tcp_client_ptr_->subcribe_data_callback(boost::bind(&StreamReceiver::tsCallback, this, _1, _2));
		ts_tcp_client_ptr_->connect();
		ts_tcp_client_ptr_->wait_response();
    }
	m3u8_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_m3u8_task, this)));
	ts_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_ts_task, this)));
    if (nullptr != m3u8_thrd_ptr_)
    {
        m3u8_thrd_ptr_->detach();
    }
    if (nullptr != ts_thrd_ptr_)
    {
        ts_thrd_ptr_->detach();
    }
	return 0;
}

void StreamReceiver::stop()
{
    m3u8_tcp_client_ptr_->unsubcribe_data_callback(m3u8_conn);
    ts_tcp_client_ptr_->unsubcribe_data_callback(ts_conn);

    if (nullptr != m3u8_thrd_ptr_)
    {
        //m3u8_thrd_ptr_->stop();
    }
    if (nullptr != ts_thrd_ptr_)
    {
        //ts_thrd_ptr_->start();
    }

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
	//std::string m3u8_name = boost::filesystem::basename(resource_path);
	ts_path.remove_filename();
	std::string ts_uri = ts_path.string() + "/" + ts_file;
	URIParser ts_uri_parser(ts_uri);
	//<< "Host:" << uri_parser_.host() << ":" << uri_parser_.port() << "\r\n"
	ss << "GET " << "/" << ts_uri_parser.resource_path() << " HTTP/1.1" << "\r\n"\
		<< "Host: " << ts_uri_parser.host() << ":" << ts_uri_parser.port() << " \r\n"\
		<< "User-Agent: " << "me-test" << "\r\n"\
		<< "Accept: " << "test/html,application/xhtml+xml,application/xml" << "\r\n"\
		<< "Connection: " << "keep-alive" << "\r\n\r\n";
	return ss.str();
}

bool StreamReceiver::_make_down_ts_cmd(HTTPTSCMD &ts_cmd, const std::string &ts_file)
{
	return true;
}

int StreamReceiver::_send_m3u8_cmd(const string &m3u8_cmd)
{
    if (nullptr != m3u8_tcp_client_ptr_)
    {
        m3u8_tcp_client_ptr_->async_send(m3u8_cmd.data(), m3u8_cmd.length());
    }
	return 0;
}

int StreamReceiver::_send_ts_cmd(const string &ts_cmd)
{
    if (nullptr != ts_tcp_client_ptr_)
    {
        ts_tcp_client_ptr_->async_send(ts_cmd.data(), ts_cmd.length());
    }
	return 0;
}

//M3U8Data StreamReceiver::_parser_m3u8_file(const char *m3u8_data, const int &length)
//{
//	M3U8Data m3u8_data_struct;
//	string data;
//	data.append(m3u8_data, length);
//	M3u8Parser parser(data);
//	std::vector<std::string> file_list;
//	parser.get_ts_file_list(file_list);
//	int index = 0;
//	for (auto &item : file_list)
//	{
//		m3u8_data_struct.ts_file_list.insert(std::make_pair(item, index));
//		index++;
//	}
//	return m3u8_data_struct;
//}

bool StreamReceiver::_parser_m3u8_file(const char *m3u8_data, const int &length, M3U8Data &m3u8_data_struct)
{
	string data;
	data.append(m3u8_data, length);
	M3u8Parser parser(data);
	std::vector<std::string> file_list;
	parser.get_ts_file_list(file_list);
	int index = 0;
	for (auto &item : file_list)
	{
		m3u8_data_struct.ts_file_list.insert(std::make_pair(item, index));
		index++;
	}
	return true;
}

int StreamReceiver::_push_ts_cmd(const string &ts_cmd_str)
{
	HTTPTSCMD ts_cmd;
	memcpy(ts_cmd.cmd, ts_cmd_str.c_str(), ts_cmd_str.length());
	ts_cmd.cmd_length = ts_cmd_str.length();
	std::map<string, int>::iterator iter = ts_all_task_map_.find(ts_cmd_str);
	if (iter == ts_all_task_map_.end())//此任务没有添加过
	{
		ts_all_task_map_.insert(std::make_pair(ts_cmd_str, ts_file_index_++));
		ts_task_list_.push(ts_cmd);
	}
	return 0;
}

bool StreamReceiver::_get_ts_cmd(HTTPTSCMD &cmd)
{
	return true;
}

void StreamReceiver::m3u8Callback(char *data, const int &data_len)
{
	//std::cout << "data:" << data << "data_len:" << data_len << endl;
	
	if (100 < ts_all_task_map_.size())//当数据过多时删除一些旧数据
	{
	}
	http_packet.append(data, data_len);
	char *end_char = strstr((char*)http_packet.c_str(), (char*)HTTP_HEAD_END.c_str());
	if (nullptr != end_char)
	{
		RegexTextFinder regex_finder;
		int index = http_packet.find_first_of("\r\n");
		int http_code = 200;
		if (index != string::npos)
		{
			string first_header_content = http_packet.substr(0, index);
			if (regex_finder.find(first_header_content, "(.+?) (\\d+) (.+$)"))
			{
				http_code = atoi(regex_finder[2].c_str());
			}
		}
		if (http_code != 200)
		{
			http_packet.clear();
			return;
		}
		int http_index = end_char + 4 - http_packet.c_str();
		if (regex_finder.find(http_packet, "Content-Length: (\\d+)"))
		{
			int content_length = atoi(regex_finder[1].c_str());//http头后的数据长度
			int one_packet_length = http_index + content_length;//一包http数据的长度
			int all_data_length = http_packet.length();//接收到数据的总长度
			if (one_packet_length <= all_data_length)
			{
				string m3u8_content = http_packet.substr(http_index, content_length);
				M3U8Data m3u8_data_struct;
				_parser_m3u8_file(m3u8_content.c_str(), m3u8_content.length(), m3u8_data_struct);
				for (auto &item : m3u8_data_struct.ts_file_list)
				{
					//cout << "file:" << item.first << "index:" << item.second << endl;
					string ts_cmd_str = _make_down_ts_cmd(item.first);
					_push_ts_cmd(ts_cmd_str);
					_write_ts_file_list(item.first, item.second);
				}
				http_packet = http_packet.substr(one_packet_length\
					, all_data_length - one_packet_length);//去除一整包http
			}
		}
	}
	return;//没有找到一整包数据
#pragma region test
	fstream out_file;
	char file_name[100] = {0};
	sprintf(file_name, "m3u8_%d.txt", ts_file_index_++);
	out_file.open(file_name, ios::out | ios::binary | ios::app);
	out_file.write(data, data_len);
	out_file.close();
    //解析mu38文件, 生成ts下载命令插入任务队列
#pragma endregion test
}

void StreamReceiver::tsCallback(char *data, const int &data_len)
{
	if (data && 0 < data_len)
	{
		char *end_char = strstr(data, (char*)HTTP_HEAD_END.c_str());
		if (nullptr != end_char)
		{
			http_ts_packet_.append(data, data_len);
			RegexTextFinder regex_finder;
			int index = http_ts_packet_.find_first_of("\r\n");
			int http_code = 200;
			if (index != string::npos)
			{
				string first_header_content = http_ts_packet_.substr(0, index);
				if (regex_finder.find(first_header_content, "(.+?) (\\d+) (.+$)"))
				{
					http_code = atoi(regex_finder[2].c_str());
				}
			}
			if (http_code != 200)
			{
				http_ts_packet_.clear();
				return;
			}
			int http_content_length = end_char - data + 4;
			int ts_send_size = data_len - http_content_length;
			if (0 < ts_send_size)
			{
				cout << "send ts data size:" << ts_send_size << std::endl;
				ts_send_signal_(end_char+4, ts_send_size);
			}
			else
			{
				cout << "end_index" << std::endl;
			}
		}
		else//不是http头
		{
			cout << "send ts data size:" << data_len << std::endl;
			ts_send_signal_(data, data_len);
		}
	}
}

int StreamReceiver::_do_m3u8_task()
{
    while (1) {
		if (0 < play_stream_.length())
            _send_m3u8_cmd(play_stream_);
        if (b_exit)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(play_stream_duration_));
    }
	return 0;
}

int StreamReceiver::_do_ts_task()
{
    while (1) {
		HTTPTSCMD ts_cmd_struct;
		while (ts_task_list_.pop(ts_cmd_struct))
		{
			string ts_cmd = ts_cmd_struct.cmd;
            if (0 < ts_cmd.length())
                _send_ts_cmd(ts_cmd);
		}
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
	return 0;

}

void StreamReceiver::_write_ts_file_list(const string &ts_file_name, const int &index)
{
	pugi::xml_node ts_file_node = ts_file_doc_.append_child("ts_file");
	pugi::xml_attribute name_attr = ts_file_node.append_attribute("name");
	name_attr.set_value(ts_file_name.c_str());
	pugi::xml_attribute index_attr = ts_file_node.append_attribute("index");
	index_attr.set_value(index);
	ts_file_doc_.save_file("ts_file.xml", "\t", pugi::encoding_utf8);
}

void StreamReceiver::_write_content_to_file(char *data, const int &data_len)
{
	fstream out_file;
	char out_file_name[100] = {0};
	sprintf(out_file_name, "out_file_%d.ts", save_content_index_);
	save_content_index_++;
	out_file.open(out_file_name, ios::out | ios::binary | ios::app);
	out_file.write(data, data_len);
	out_file.close();
}

