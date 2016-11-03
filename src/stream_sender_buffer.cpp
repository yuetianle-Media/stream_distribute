#include "stdafx.h"
#include "stream_sender_buffer.h"


StreamSenderBuffer::StreamSenderBuffer()
	:cur_data_curosor_(data_), cur_data_len_(0), remain_data_len_(sizeof(data_))
{
}


StreamSenderBuffer::~StreamSenderBuffer()
{
}

int StreamSenderBuffer::push_to_buffer(char* data, const long int &data_len)
{
	if (!data || 0 > data_len)
	{
		return E_DATA_EMPTY;
	}
	if (data_len > remain_data_len_)
	{
		return E_BUFFER_LESS;
	}
	memcpy(cur_data_curosor_, data, data_len);
	//memmove(cur_data_curosor_, data, data_len);
	cur_data_curosor_ += data_len;
	remain_data_len_ -= data_len;
	cur_data_len_ += data_len;
	return E_OK;
}

char* StreamSenderBuffer::get_data()
{
	return data_;
}

int StreamSenderBuffer::get_data_length()
{
	return cur_data_len_;
}

int StreamSenderBuffer::reset_buffer()
{
	memset(data_, 0, sizeof(data_));
	cur_data_len_ = 0;
	cur_data_curosor_ = data_;
	remain_data_len_ = sizeof(data_);
	return 0;
}
