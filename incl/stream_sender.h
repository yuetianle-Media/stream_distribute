
#pragma once
#include "udpclient.h"
#include "stream_buffer.h"

//class TSParser;
class StreamSender
{
public:
	StreamSender();
	~StreamSender();

	void stream_receive_callback(char *data, const int &data_len);
private:
	UDPClientPtr udp_sender_ptr;
	//TSParser ts_parser_;
	StreamBuffer stream_buffer_;
};