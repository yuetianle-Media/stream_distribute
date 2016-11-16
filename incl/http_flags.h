#ifndef _HTTP_FLAGS_H_
#define _HTTP_FLAGS_H_
#pragma once

#include "pre_std_basic.h"

const std::string HTTP_HEAD_END = "\r\n\r\n";
const std::string HTTP_CONTENT_LENGTH = "Content-Length";
const std::string HTTP_RESPONSE_START_FLAG = "HTTP";

typedef enum HttpCMD
{
	HttpM3u8 = 0,
	HttpTs = 1,
}VHttp_t;

struct HTTPM3U8CMD
{
	char cmd[1024];
	int cmd_length;
	HTTPM3U8CMD()
	{
		memset(cmd, 0, sizeof(HTTPM3U8CMD));
	}
};
struct HTTPTSCMD
{
	char cmd[300];
	int cmd_length;
	HTTPTSCMD()
	{
		memset(cmd, 0, sizeof(HTTPTSCMD));
	}
};
#endif