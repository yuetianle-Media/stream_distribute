#ifndef EQAM_DEFINE_H
#define EQAM_DEFINE_H

#include "ztypes.h"
#include <time.h>

#include <string>
#include <sstream>
using namespace std;


typedef unsigned short PID;
#define INVALID_PID 0xffff
#define MAX_PID 	0x1fff

#define TS_PACKET_IDENT	0x47

typedef long long PCR;
#define INVALID_PCR (PCR)(-1)
#define MAX_PCR 0x40000000000LL

#define TS_PACKET_LENGTH_STANDARD		188


#define TS_REF_CLOCK	27000000

#define DEFAULT_TS_PCR_INTERVAL (TS_REF_CLOCK / 20)

#define MS_TO_PCR(time_ms) (time_ms * 27000)

typedef unsigned short PROGRAM_NUMBER;
typedef unsigned char SECTION_NUMBER;

#define GET_TS_PID(pBuf) (WORD)((((*(pBuf)) & 0x1f)<<8) | (*(pBuf+1)))

#define GET_PCR_INTERVAL(from,to) ((from <= to) ? (to-from) : (MAX_PCR - from + to))

//为了排序要求，反向结构
//union VOD_TIMECODE_T
//{
//	struct
//	{
//		BYTE	sec_1_250;
//		BYTE	sec;
//		BYTE	min;
//		BYTE	hour;
//		BYTE	day;
//		BYTE	mon;
//		short	year;
//	}tm;
//	LONGLONG	tm_value;
//};

//time_t VodTime2SysTime(const VOD_TIMECODE_T &VodTime);
//VOD_TIMECODE_T SysTime2VodTime(const time_t &SysTime);
//string FormatVodTime(const VOD_TIMECODE_T &VodTime);


#endif

