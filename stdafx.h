// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"
#pragma warning(disable : 4496)
#ifdef win32
#include <tchar.h>
#include <stdio.h>
#endif // Win32

#ifdef WIN32
#define _ENABLE_ATOMIC_ALIGNMENT_FIX 
#endif // WIN32

#define ENABLE_LOG 1
#ifdef ENABLE_LOG
#include "vistek_logger.h"
#ifndef __LOG_SOURCE__
#define __LOG_SOURCE__ "stream_distribute"
#endif
#endif 


// TODO:  在此处引用程序需要的其他头文件
