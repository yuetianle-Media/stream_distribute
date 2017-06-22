#pragma once

#ifndef _BASE_RECEIVER_H_
#define _BASE_RECEIVER_H_

#include "pre_boost_basic.h"
#include "stream_buffer.h"

#include "tspacket.h"
#include "stream_sender_buffer.h"

#include "thread_pool.h"
#ifdef __linux
#include <error.h>
#endif
#include "data_types_defs.h"

class BaseReceiver: public std::enable_shared_from_this<BaseReceiver>
{
public:
	BaseReceiver();
	~BaseReceiver();
public:
	virtual int start()=0;
	virtual int stop()=0;
	virtual bool get_ts_send_data_queue(TSSendSpscQueueType *&send_queue);
private:
	TSSendSpscQueueType ts_send_queue_;
	

};

#endif
