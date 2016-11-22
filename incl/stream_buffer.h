/* Copyright(C)
 * For free
 * All right reserved
 *
 */

#pragma once
#include "errcode.h"
#include "pre_std_basic.h"
using namespace std;
#define v_lock(l, m) std::lock_guard<decltype((m))> (l)((m))
#define BUFFER_MAX_SIZE 262144/*<< 0.5M*/
class StreamBuffer: public std::enable_shared_from_this<StreamBuffer>
{
    public:
        StreamBuffer(const long int &size);
        ~StreamBuffer();

        int pushToBuffer(const char *content, const int &size);
		bool  pop_data(char *dest, const int &dest_len, const int &data_len);
		bool is_empty();
        char* data() { return (char*)data_content_; }
		const long int data_len() { v_lock(lk, mtx_data_); return data_size_; }
    private:
        std::atomic<int> current_index_;
        std::atomic<int> data_size_;
		char data_content_[BUFFER_MAX_SIZE];
		std::mutex mtx_data_;
		long int capacity_;
};

typedef std::shared_ptr<StreamBuffer> StreamBufferPtr;
