#include "stdafx.h"
#include "stream_buffer.h"

StreamBuffer::StreamBuffer(const int &size)
    :current_index_(-1), data_size_(size)
{

}

StreamBuffer::~StreamBuffer()
{

}

int StreamBuffer::pushToBuffer(const char *content, const int &size)
{
    return 0;
}

