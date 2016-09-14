#include "stdafx.h"
#include "stream_receiver.h"


StreamReceiver::StreamReceiver(const string &url)
	:play_stream_(url), ts_task_list_(0)
{
	m3u8_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_m3u8_task, this)));
	ts_thrd_ptr_.reset(new std::thread(std::bind(&StreamReceiver::_do_ts_task, this)));

}

StreamReceiver::~StreamReceiver()
{

}

int StreamReceiver::start()
{
    if (nullptr != m3u8_tcp_client_ptr_)
    {
        m3u8_conn =  m3u8_tcp_client_ptr_->subcribe_data_callback(boost::bind(&StreamReceiver::m3u8Callback, this, _1, _2));
		m3u8_tcp_client_ptr_->send(play_stream_.data(), play_stream_.length());
    }
    if (nullptr != ts_tcp_client_ptr)
    {
        ts_conn = ts_tcp_client_ptr->subcribe_data_callback(boost::bind(&StreamReceiver::tsCallback, this, _1, _2));
    }
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
    ts_tcp_client_ptr->unsubcribe_data_callback(ts_conn);

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

std::string StreamReceiver::_make_m3u8_cmd(const string &stream_url)
{
	return "";
}

std::string StreamReceiver::_make_down_ts_cmd(const string &ts_file)
{
	return "";
}

int StreamReceiver::_send_m3u8_cmd(const string &m3u8_cmd)
{
    if (nullptr != m3u8_tcp_client_ptr_)
    {
        m3u8_tcp_client_ptr_->send(m3u8_cmd.data(), m3u8_cmd.length());
    }
	return 0;
}

int StreamReceiver::_send_ts_cmd(const string &ts_cmd)
{
    if (nullptr != ts_tcp_client_ptr)
    {
        ts_tcp_client_ptr->send(ts_cmd.data(), ts_cmd.length());
    }
	return 0;
}

void StreamReceiver::m3u8Callback(char *data, const int &data_len)
{
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
		TSCMD ts_cmd_struct;
		while (ts_task_list_.pop(ts_cmd_struct))
		{
			//string ts_cmd = ts_cmd_struct.cmd;
			string ts_cmd;
            if (0 < ts_cmd.length())
                _send_ts_cmd(ts_cmd);
		}
        std::this_thread::sleep_for(std::chrono::milliseconds(play_stream_duration_));
    }
	return 0;

}

