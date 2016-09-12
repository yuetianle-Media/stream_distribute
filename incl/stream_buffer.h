/* Copyright(C)
 * For free
 * All right reserved
 *
 */

#include <iostream>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

class StreamBuffer: public boost::enable_shared_from_this<StreamBuffer>
{
    public:
        StreamBuffer(const int &size);
        ~StreamBuffer();

        int pushToBuffer(const char *content, const int &size);

    private:
        int current_index_;
        int data_size_;
        std::vector<char> data_;
};

typedef boost::shared_ptr<StreamBuffer> StreamBufferPtr;
