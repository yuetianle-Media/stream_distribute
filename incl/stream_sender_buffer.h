#pragma once
#include "errcode.h"

class StreamSenderBuffer
{
public:
	StreamSenderBuffer();
	~StreamSenderBuffer();

	int push_to_buffer(char* data, const long int &data_len);
	char* get_data();
	int get_data_length();
	int reset_buffer();

private:
	char data_[188 * 7 * 200];
	char *cur_data_curosor_;
	atomic<long int> cur_data_len_;
	atomic<long int> remain_data_len_;
};

