#include "stdafx.h"
#include "stream_receiver.h"

StreamReceiver::StreamReceiver(const string &url)
{

}

StreamReceiver::~StreamReceiver()
{

}

int StreamReceiver::start()
{
	return 0;
}

void StreamReceiver::stop()
{

}

void StreamReceiver::subcribe_m3u8_callback()
{

}

void StreamReceiver::subcribe_ts_callback()
{

}

std::string StreamReceiver::_make_m3u8_cmd()
{
	return "";
}

std::string StreamReceiver::_make_down_ts_cmd()
{
	return "";
}

int StreamReceiver::_send_m3u8_cmd(const string &m3u8_cmd)
{
	return 0;
}

int StreamReceiver::_send_ts_cmd(const string &ts_cmd)
{
	return 0;
}

void StreamReceiver::m3u8Callback(char *data, const int &data_len)
{

}

void StreamReceiver::tsCallback(char *data, const int &data_len)
{

}

int StreamReceiver::_do_m3u8_task()
{
	return 0;

}

int StreamReceiver::_do_ts_task()
{
	return 0;

}

