// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
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


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
