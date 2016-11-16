#pragma once

#include "pre_boost_basic.h"
#include "pre_std_basic.h"

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
	PCR cur_pcr;
	bool is_real_pcr;
	TS_SEND_CONTENT()
		:real_size(0), need_time(0), cur_pcr(0), is_real_pcr(false)
	{
		memset(content, 0, sizeof(TS_SEND_SIZE));
	}
}TSSENDCONTENT;

typedef boost::lockfree::queue<TSSENDCONTENT, boost::lockfree::fixed_sized<false>> TSSendUnlimitQueueType;
typedef boost::lockfree::queue<TSSENDCONTENT, boost::lockfree::fixed_sized<true>> TSSendQueueType;
typedef boost::lockfree::spsc_queue<TSSENDCONTENT, boost::lockfree::capacity<1024 * 5>> TSSendSpscQueueType;

typedef struct TSPACKETCONTENT
{
	char content[TS_PACKET_LENGTH_STANDARD];
	int real_size;
	PCR pcr;
	TSPACKETCONTENT()
		:real_size(0), pcr(0)
	{
		memset(content, 0, sizeof(content));
	}
}TS_PACKET_CONTENT;//一个ts包

typedef boost::lockfree::queue<TS_PACKET_CONTENT, boost::lockfree::fixed_sized<true>> TSPacketQueueType;
typedef boost::lockfree::spsc_queue<TS_PACKET_CONTENT, boost::lockfree::capacity<32000>> TSPacketSpscQueueType;
typedef boost::lockfree::queue<HTTPTSCMD, boost::lockfree::fixed_sized<true>> TSTaskType;

using TSTaskTypePtr = std::shared_ptr<TSTaskType>;
using TSTaskGroup = std::map<int, TSTaskTypePtr>;

typedef boost::signals2::signal<void(char *, const long int&, const bool&)> TsSignal;

const long int M3U8TASKREMOVEINTERVAL = 60 * 2;/*unit:s*/


