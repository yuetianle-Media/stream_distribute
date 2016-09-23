/* Copyright(C)
 * For free
 * All right reserved
 *
 */

#pragma once
using namespace std;
#include "errcode.h"
#include "pre_std_basic.h"
#define v_lock(l, m) std::lock_guard<decltype((m))> (l)((m))
class StreamBuffer: public std::enable_shared_from_this<StreamBuffer>
{
    public:
        StreamBuffer(const long int &size);
        ~StreamBuffer();

        int pushToBuffer(const char *content, const int &size);
		bool  pop_data(char *dest, const int &dest_len, const int &data_len);
		bool is_empty();
    private:
        std::atomic<int> current_index_;
        std::atomic<int> data_size_;
        std::vector<char> data_;
		std::mutex mtx_data_;
};

typedef std::shared_ptr<StreamBuffer> StreamBufferPtr;
