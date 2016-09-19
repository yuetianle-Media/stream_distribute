#include "stdafx.h"
#include "stream_receiver.h"


StreamReceiver::StreamReceiver(const string &url)
	:ts_task_list_(0), uri_parser_(url), play_stream_duration_(5)\
	, m3u8_thrd_ptr_(nullptr), ts_thrd_ptr_(nullptr), b_exit(false)
{
	play_stream_ = _make_m3u8_cmd(url);
	m3u8_tcp_client_ptr_	= std::make_shared<TCPClient>(uri_parser_.host(), uri_parser_.port(), 5000);
	//ts_tcp_client_ptr_		= std::make_shared<TCPClient>(uri_parser_.host(), uri_parser_.port(), 5000);

}

StreamReceiver::~StreamReceiver()
{

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
		//m3u8_tcp_client_ptr_->wait_response();
    }
    if (nullptr != ts_tcp_client_ptr_)
    {
        ts_conn = ts_tcp_client_ptr_->subcribe_data_callback(boost::bind(&StreamReceiver::tsCallback, this, _1, _2));
    }
	m3u8_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_m3u8_task, this)));
	//ts_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_ts_task, this)));
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
	ss << "GET " << "/" <<uri_parser_.resource_path() << " HTTP/1.1" << "\r\n"\
		<< "Host:" << uri_parser_.host() << ":" << uri_parser_.port() << " \r\n"\
		<< "User-Agent:" << "me-test" << "\r\n"\
		<< "Accept:" << "test/html,application/xhtml+xml,application/xml" << "\r\n"\
		<< "Connection:" << "keep-alive" << "\r\n";
	return ss.str();
}

std::string StreamReceiver::_make_down_ts_cmd(const string &ts_file)
{
	std::stringstream ss;
	boost::filesystem::path resource_path(uri_parser_.uri());
	//std::string m3u8_name = boost::filesystem::basename(resource_path);
	resource_path.remove_filename();
	resource_path /= ts_file;
	//<< "Host:" << uri_parser_.host() << ":" << uri_parser_.port() << "\r\n"
	ss << "GET " <<  resource_path << "HTTP/1.1" << "\r\n"\
		<< "User-Agent:" << "me-test" << "\r\n"\
		<< "Accept:" << "test/html,application/xhtml+xml,application/xml" << "\r\n"\
		<< "Connection:" << "keep-alive" << "\r\n";
	return "";
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
        ts_tcp_client_ptr_->send(ts_cmd.data(), ts_cmd.length());
    }
	return 0;
}

void StreamReceiver::m3u8Callback(char *data, const int &data_len)
{
	std::cout << "data:" << data << "data_len:" << data_len << endl;
    //解析mu38文件, 生成ts下载命令插入任务队列
}

void StreamReceiver::tsCallback(char *data, const int &data_len)
{
    ts_send_signal_(data, data_len);
}

int StreamReceiver::_do_m3u8_task()
{
    while (1) {
		if (0 < play_stream_.length())
			m3u8_tcp_client_ptr_->receive();
            //_send_m3u8_cmd(play_stream_);
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
		TSCMD ts_cmd_struct;
		while (ts_task_list_.pop(ts_cmd_struct))
		{
			string ts_cmd = ts_cmd_struct.cmd;
			//string ts_cmd;
            if (0 < ts_cmd.length())
                _send_ts_cmd(ts_cmd);
		}
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
	return 0;

}

