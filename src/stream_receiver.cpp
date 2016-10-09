#include "stdafx.h"
#include "stream_receiver.h"


StreamReceiver::StreamReceiver(const string &url)
	:ts_task_list_(10000), uri_parser_(url), play_stream_duration_(5)\
	, m3u8_thrd_ptr_(nullptr), ts_thrd_ptr_(nullptr), b_exit(false)\
	, ts_file_index_(0), callback_times_(0), save_content_index_(0)\
	, is_receive_ts_response_header_(false), ts_response_content_length_(0)
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
		//std::cout << "m3u8 and ts thread is started m3u8 thread id:" << m3u8_thrd_ptr_->get_id()\
			<< "ts thread id:" << ts_thrd_ptr_->get_id() << endl;
		vvlog_w("m3u8 and ts thread is started m3u8 thread id:" << m3u8_thrd_ptr_->get_id()\
			<< "ts thread id:" << ts_thrd_ptr_->get_id());
		return E_OK;
	}
    if (nullptr != m3u8_tcp_client_ptr_)
    {
        m3u8_conn =  m3u8_tcp_client_ptr_->subcribe_data_callback(boost::bind(&StreamReceiver::m3u8Callback, this, _1, _2));
		//m3u8_tcp_client_ptr_->send(play_stream_.data(), play_stream_.length());
		m3u8_tcp_client_ptr_->connect();
		m3u8_tcp_client_ptr_->wait_response();
		m3u8_tcp_client_ptr_->async_send(play_stream_.data(), play_stream_.length());
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

bool StreamReceiver::_find_http_header_start(char* &dest, char*src, const int src_length)
{
	char *tmp_src = src;
	int tmp_length = src_length;
	while (tmp_src && 4 <= tmp_length)
	{
		if ('H' == *tmp_src && 'T' == *(tmp_src + 1) && 'T' == *(tmp_src + 2) && 'P' == *(tmp_src + 3))//"HTTP"
		{
			dest = tmp_src;
			return true;
		}
		if ('h' == *tmp_src && 't' == *(tmp_src + 1) && 't'== *(tmp_src + 2) && 'p' == *(tmp_src + 3))//"http"
		{
			dest = tmp_src;
			return true;
		}
		tmp_src++;
		tmp_length--;
	}
	return false;
}

bool StreamReceiver::_find_http_header_end(char* &dest, char*src, const int src_length)
{
	char *tmp_src = src;
	int tmp_length = src_length;
	while (tmp_src && 4 <= tmp_length)
	{
		if ('\r'== *tmp_src && '\n'== *(tmp_src + 1) && '\r'== *(tmp_src + 2) && '\n' == *(tmp_src + 3))
		{
			dest = tmp_src+4;
			return true;
		}
		tmp_src++;
		tmp_length--;
	}
	return false;
}

bool StreamReceiver::_find_http_line_end(char* &dest, char*src, const int src_length)
{
	char *tmp_src = src;
	int tmp_length = src_length;
	while (tmp_src && 4 <= tmp_length)
	{
		if ('\r'== *tmp_src && '\n'== *(tmp_src + 1))
		{
			dest = tmp_src+2;
			return true;
		}
		tmp_src++;
		tmp_length--;
	}
	return false;
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
		vvlog_i("send m3u8 cmd:" << m3u8_cmd);
        m3u8_tcp_client_ptr_->async_send(m3u8_cmd.data(), m3u8_cmd.length());
    }
	return 0;
}

int StreamReceiver::_send_ts_cmd(const string &ts_cmd)
{
    if (nullptr != ts_tcp_client_ptr_)
    {
		vvlog_i("send ts start cmd:" << ts_cmd);
		//ts_tcp_client_ptr_->async_send_ext(ts_cmd.data(), ts_cmd.length());
        ts_tcp_client_ptr_->async_send(ts_cmd.data(), ts_cmd.length());
		vvlog_i("send ts end.");
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
		m3u8_data_struct.ts_file_list.push_back(item);
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
		vvlog_i("push ts task:" << ts_cmd_str);
		_write_ts_file_list("task.xml", ts_cmd_str, 0);
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
	//char http_out_file_name[1024] = { 0 };
	//sprintf(http_out_file_name, "http_out_file_%d.ts", save_content_index_);
	//_write_content_to_file(http_out_file_name, data, data_len);
	//std::cout << "data:" << data << "data_len:" << data_len << endl;
	//vvlog_i("m3u8 callback data:" <<data << "datalen:"<< data_len);
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
			vvlog_w("m3u8 callback error:" << http_code);
			http_packet.clear();
			return;
		}
		int http_head_length = end_char + 4 - http_packet.data();
		if (http_head_length == data_len)//只有HTTP头
			return;
		if (regex_finder.find(http_packet, "Content-Length: (\\d+)"))
		{
			int content_length = atoi(regex_finder[1].c_str());//http头后的数据长度
			int one_packet_length = content_length + http_head_length;
			if (one_packet_length <= http_packet.length())
			{
				string m3u8_content = http_packet.substr(http_head_length, content_length);
				M3U8Data m3u8_data_struct;
				_parser_m3u8_file(m3u8_content.c_str(), m3u8_content.length(), m3u8_data_struct);
				char out_file_name[1024] = {0};
				sprintf(out_file_name, "m3u8callbackfile_%d.xml", callback_times_);
				int file_index = 0;
				for (auto &item : m3u8_data_struct.ts_file_list)
				{
					//cout << "file:" << item.first << "index:" << item.second << endl;
					string ts_cmd_str = _make_down_ts_cmd(item);
					_push_ts_cmd(ts_cmd_str);
					//_write_ts_file_list(out_file_name, item, file_index);
					file_index++;
				}
				//callback_times_++;
				http_packet = http_packet.substr(one_packet_length\
					, http_packet.length() - one_packet_length);//去除一整包http
				return;
			}
		}
		//int http_index = end_char + 4 - http_packet.c_str();
		//if (regex_finder.find(http_packet, "Content-Length: (\\d+)"))
		//{
		//	int content_length = atoi(regex_finder[1].c_str());//http头后的数据长度
		//	int one_packet_length = http_index + content_length;//一包http数据的长度
		//	int all_data_length = http_packet.length();//接收到数据的总长度
		//	if (one_packet_length <= all_data_length)
		//	{
		//		string m3u8_content = http_packet.substr(http_index, content_length);
		//		M3U8Data m3u8_data_struct;
		//		_parser_m3u8_file(m3u8_content.c_str(), m3u8_content.length(), m3u8_data_struct);
		//		char out_file_name[1024] = {0};
		//		sprintf(out_file_name, "m3u8callbackfile_%d.xml", callback_times_);
		//		for (auto &item : m3u8_data_struct.ts_file_list)
		//		{
		//			//cout << "file:" << item.first << "index:" << item.second << endl;
		//			string ts_cmd_str = _make_down_ts_cmd(item.first);
		//			_push_ts_cmd(ts_cmd_str);
		//			_write_ts_file_list(out_file_name, item.first, item.second);
		//		}
		//		callback_times_++;
		//		http_packet = http_packet.substr(one_packet_length\
		//			, all_data_length - one_packet_length);//去除一整包http
		//	}
		//}
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
#pragma region new
	_write_content_to_file("ts_callbcak.dat", data, data_len);
	char *http_header_start = nullptr;
	char *http_header_end = nullptr;
	int send_length = 0;
	vvlog_i("ts_callback start length:" << data_len);
	_find_http_header_start(http_header_start, data, data_len);
	_find_http_header_end(http_header_end, data, data_len);
	if (http_header_start)
	{
		int head_length = http_header_start - data;
		int http_code = 0;
		char *http_first_line = nullptr;
		if (_find_http_line_end(http_first_line, data, data_len))
		{
			char first_line_content[1024] = { 0 };
			if (http_first_line)
			{
				int first_line_length = http_first_line - http_header_start;
				if (0 < first_line_length)
					memcpy(first_line_content, http_header_start, first_line_length);
			}
			if (0 < strlen(first_line_content))
			{
				RegexTextFinder regex_finder;
				if (regex_finder.find(first_line_content, "(.+?) (\\d+) (.+$)"))
				{
					http_code = atoi(regex_finder[2].c_str());
				}
			}
			if (200 != http_code)//如果http请求失败不进行数据传输
			{
				return;
			}
		}
		if (0 < head_length)
		{
			send_length = head_length;
			if (0 < send_length)
			{
				ts_send_signal_(data, head_length);
				vvlog_i("ts_callback send data len:" << send_length);
			}
		}
		if (http_header_end)
		{
			int http_header_length = http_header_end - data;
			send_length = data_len - http_header_length;
			if (0 < send_length)
			{
				ts_send_signal_(data + http_header_length, send_length);
				vvlog_i("ts_callback send data len:" << send_length);
			}
		}
		else
		{
			vvlog_w("only find header start not header end!!!");
		}
	}
	else
	{
		if (http_header_end)
		{
			int http_end_length = http_header_end - data;
			send_length = data_len - http_end_length;
			if (0 < send_length)
			{
				ts_send_signal_(data + http_end_length, send_length);
				vvlog_i("ts_callback send data len:" << send_length);
			}
		}
		else
		{
			send_length = data_len;
			if (0 < send_length)
			{
				ts_send_signal_(data, data_len);
				vvlog_i("ts_callback send data len:" << send_length);
			}
		}
	}
	vvlog_i("ts_callback end");
	return;
#pragma endregion new
	vvlog_i("ts callback start datalen:" << data_len);
	//char http_out_file_name[1024] = { 0 };
	//sprintf(http_out_file_name, "http_out_file_%d.txt", save_content_index_);
	//_write_content_to_file(http_out_file_name, data, data_len);
	if (data && 0 < data_len)
	{
		callback_times_++;
		char out_file_name[1024] = { 0 };
		sprintf(out_file_name, "send_ts_%d.ts", callback_times_);
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
				//cout << "ts call callback error len:" << data_len << "data:" << data << std::endl;
				vvlog_w("ts call callback error len:" << data_len << "data:" << data);
				http_ts_packet_.clear();
				return;
			}
			int http_content_length = end_char - data + 4;
			int ts_send_size = data_len - http_content_length;
			if (0 < ts_send_size)
			{
				//cout << "send ts data size:" << ts_send_size << std::endl;
				vvlog_i("send ts data size:" << ts_send_size);
				_write_content_to_file(out_file_name, end_char + 4, ts_send_size);
				ts_send_signal_(end_char+4, ts_send_size);
			}
			else
			{
				vvlog_i("end_index ts_send_size:" << ts_send_size);
				//cout << "end_index ts_send_size:" << ts_send_size << std::endl;
			}
			http_ts_packet_.clear();
		}
		else//不是http头
		{
			vvlog_i("send ts data size:" << data_len);
			//cout << "send ts data size:" << data_len << std::endl;
			_write_content_to_file(out_file_name, data, data_len);
			ts_send_signal_(data, data_len);
		}
	}
	vvlog_i("ts callback end datalen:" << data_len);
}

int StreamReceiver::_do_m3u8_task()
{
    while (1) {
		if (0 < play_stream_.length())
		{
            _send_m3u8_cmd(play_stream_);
			std::this_thread::sleep_for(std::chrono::seconds(play_stream_duration_));
		}
        if (b_exit)
        {
            break;
        }
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
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
	return 0;

}

void StreamReceiver::_write_ts_file_list(const string &out_file_name, const string &ts_file_name, const int &index)
{
	pugi::xml_node ts_file_node = ts_file_doc_.append_child("ts_file");
	pugi::xml_attribute name_attr = ts_file_node.append_attribute("name");
	name_attr.set_value(ts_file_name.c_str());
	pugi::xml_attribute index_attr = ts_file_node.append_attribute("index");
	index_attr.set_value(index);
	char out_file[1024] = {0};
	ts_file_doc_.save_file(out_file_name.data(), "\t", pugi::encoding_utf8);
}

void StreamReceiver::_write_content_to_file(const string &out_file_name, char *data, const int &data_len)
{
	fstream out_file;
	//char out_file_name[100] = {0};
	//sprintf(out_file_name, "out_file_%d.ts", save_content_index_);
	save_content_index_++;
	out_file.open(out_file_name.data(), ios::out | ios::binary | ios::app);
	out_file.write(data, data_len);
	out_file.close();
}

