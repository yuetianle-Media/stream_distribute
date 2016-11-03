#pragma once
#include <boost/lockfree/queue.hpp>
#include <stdio.h> 
#include <string.h>
#include "dvb.h"
#include "http_flags.h"
#ifndef TS_SEND_SIZE
//此值得设置和MTU相关
#define TS_SEND_SIZE 188*7
#endif
typedef struct TS_SEND_CONTENT
{
	char content[TS_SEND_SIZE];
	int real_size;
	double need_time;
	TS_SEND_CONTENT()
		:real_size(0), need_time(0)
	{
		memset(content, 0, sizeof(TS_SEND_SIZE));
	}
}TSSENDCONTENT;

//typedef boost::lockfree::queue<TSSENDCONTENT, boost::lockfree::fixed_sized<false>> TSSendQueueType;
typedef boost::lockfree::queue<TSSENDCONTENT, boost::lockfree::fixed_sized<true>> TSSendQueueType;

typedef struct TSPACKETCONTENT
{
	char content[TS_PACKET_LENGTH_STANDARD];
	int real_size;
	TSPACKETCONTENT()
		:real_size(0)
	{
		memset(content, 0, sizeof(content));
	}
}TS_PACKET_CONTENT;//一个ts包

typedef boost::lockfree::queue<TS_PACKET_CONTENT, boost::lockfree::fixed_sized<true>> TSPacketQueueType;

typedef boost::lockfree::queue<HTTPTSCMD, boost::lockfree::fixed_sized<true>> TSTaskType;

using TSTaskTypePtr = std::shared_ptr<TSTaskType>;
using TSTaskGroup = std::map<int, TSTaskTypePtr>;

typedef boost::signals2::signal<void(char *, const long int&, const bool&)> TsSignal;

const long int M3U8TASKREMOVEINTERVAL = 60 * 2;/*unit:s*/


