#include "stdafx.h"
#include "stream_sender.h"
StreamSender::StreamSender()
	:udp_sender_ptr(nullptr), stream_buffer_(0)
{

}

StreamSender::~StreamSender()
{

}

void StreamSender::stream_receive_callback(char *data, const int &data_len)
{

}

