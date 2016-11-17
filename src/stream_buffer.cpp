#include "stdafx.h"
#include "stream_buffer.h"
#include <assert.h>
#include <string.h>
StreamBuffer::StreamBuffer(const long int &size)
    :current_index_(0), data_size_(0), capacity_(size)
{
	memset(data_content_, 0, BUFFER_MAX_SIZE);
}

StreamBuffer::~StreamBuffer()
{

}

int StreamBuffer::pushToBuffer(const char *content, const int &size)
{
	v_lock(lk, mtx_data_);
	if (content && 0 < size && (BUFFER_MAX_SIZE-current_index_)>size)
	{
		memcpy(data_content_+current_index_, content, size);
		current_index_ += size;
		data_size_ += size;
		return E_OK;
	}
	return E_BUFFER_FULL;
}

bool StreamBuffer::pop_data(char *dest, const int &dest_len, const int &data_len)
{
	v_lock(lk, mtx_data_);
	if (is_empty())
	{
		return false;
	}
	if (data_content_[0] != 0x47)
	{
		assert(false);
	}
	//将代取出的数据拷贝出去
	if (dest && dest_len >= data_len &&data_size_ >= data_len)
	{
#ifdef _WIN32
		memcpy_s(dest, dest_len, data_content_, data_len);
#else
		memcpy(dest, (char*)data_content_, data_len);
#endif
		//将现有数据向前移动data_len个字节
		memmove((char*)data_content_, (char*)(data_content_ + data_len), data_size_ - data_len);
		data_size_		-= data_len;
		current_index_	-= data_len;
		return true;
	}
	else
	{
		assert(true);
	}
	return false;
}

bool StreamBuffer::is_empty()
{
	if (0 < data_size_)
		return false;
	return true;
}

