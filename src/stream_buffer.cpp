#include "stdafx.h"
#include "stream_buffer.h"
#include <assert.h>
StreamBuffer::StreamBuffer(const long int &size)
    :current_index_(0), data_size_(0), data_(size)
{

}

StreamBuffer::~StreamBuffer()
{

}

int StreamBuffer::pushToBuffer(const char *content, const int &size)
{
	if (content && 0 < size)
	{
		v_lock(lk, mtx_data_);
		memcpy(data_.data()+current_index_, content, size);
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
	//将代取出的数据拷贝出去
	if (dest && dest_len >= data_len &&data_size_ >= data_len)
	{
		memcpy_s(dest, dest_len, (char*)data_.data(), data_len);
		//将现有数据向前移动data_len个字节
		memmove((char*)data_.data(), (char*)(data_.data() + current_index_), data_size_ - data_len);
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

