#ifndef ZTYPES_H
#define ZTYPES_H

/**
 * @file ztypes.h
 * @brief 提供通用类型定义的头文件。
 */
#pragma once
#include <ctype.h>
#include <vector>
#include <typeinfo>
#include <string.h>
#include <stdlib.h>
using namespace std;

/**
 * @typedef UINT
 * @brief 无符号32位整形
 */
typedef unsigned int 			UINT ;

/**
 * @typedef LONGLONG
 * @brief 有符号64位整形
 */
typedef long long		 		LONGLONG;
/**
 * @typedef ULONGLONG
 * @brief 有符号64位整形
 */
typedef unsigned long long 		ULONGLONG;
/**
 * @typedef BYTE
 * @brief 无符号整形，8 位，字节类型
 */
typedef unsigned char			BYTE;
/**
 * @typedef WORD
 * @brief 无符号16位整形，16 位
 */
#ifndef WORD
typedef unsigned short 			WORD;
#endif
/**
 * @typedef DWORD
 * @brief 无符号32位整形
 */
#ifdef _WIN32
#include <Windows.h>
#else

typedef unsigned int				DWORD;
#endif // _WIN32



/**
 * @typedef VEC_HEX_T
 * @brief 定义二进制类型
 */
typedef vector<unsigned char>    VEC_HEX_T;

/**
 * @typedef SOCKET_T
 * @brief 定义SOCKET类型
 */
typedef int		SOCKET_T;
/**
 * @typedef EPOLL_T
 * @brief 定义EPOLL 句柄类型
 */
typedef int  	EPOLL_T;
#define ZNULL ((void *)0)

#endif

