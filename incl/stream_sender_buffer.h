#pragma once
#include <atomic>
#include <string.h>
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
#ifdef WIN32
	char data_[188 * 7 * 400];
#else
	char data_[188 * 7 * 400];
#endif // _WIN32
	char *cur_data_curosor_;
	std::atomic<long int> cur_data_len_;
	std::atomic<long int> remain_data_len_;
};

