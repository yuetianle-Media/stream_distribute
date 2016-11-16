
/**
 * @file tcpclient.h
 * @brief tcp Client.
 * @author lee, shida23577@hotmail.com
 * @version 1.0.1
 * @date 2016-09-27
 */
#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_
#pragma once
#include <stdexcept>
#include "socket_session.h"
#include "http_flags.h"

//using namespace boost;
typedef boost::signals2::signal<void(char* data, const int& data_len)> DataReceiveSignal;

/*
 *
 * example:
 * TCPClient client("172.16.1.1", 8000, 5000);
 * client.connect()
 * client.wait_response();
 * function<char*, int> callback;
 * client.subcribe_data_callback(callback,_1,_2);
 * char data[1024] = {0};
 * int data_len = sizeof(data);
 * client.async_send(data, data_len);
 *
 * */


class TCPClient: public SocketSession
{
public:
	TCPClient(const string &remote_server, const int &port, const int &time_out/*ms*/);
    ~TCPClient();

    /**
     * @brief send 同步发送数据.
     *
     * @param content
     * @param length
     *
     * @returns 0 for success else error code reference errcode.h.
     */
    virtual int send (const char *content, const int &length)override;

    /**
     * @brief receive 同步接收数据.
     *
     * @returns 0 for success else error code reference errcode.h.
     */
    virtual int receive()override;

    /**
     * @brief async_send 异步发送数据.
     *
     * @param content type:char*
     * @param length type:int
     *
     * @returns 0 for success else error code reference errcode.h.
     */
	virtual int async_send(const char *content, const int &length)override;
	/*
	*/
	virtual int async_send_ext(const char *content, const int &length);
    /**
     * @brief async_receive
     *
     * @returns
     */
	virtual int async_receive()override;


    /**
     * @brief wait_response 异步等待结果.
     *
     * @returns
     */
	virtual int wait_response();

    /**
     * @brief subcribe_data_callback 订阅数据回调.
     *
     * @param slot 数据回调函数.
     *
     * @returns 订阅连接.
     */
    boost::signals2::connection subcribe_data_callback(const DataReceiveSignal::slot_type &slot);

    /**
     * @brief unsubcribe_data_callback 取消订阅.
     *
     * @param subcriber 订阅连接.
     */
    void unsubcribe_data_callback(boost::signals2::connection &subcriber);

private:

    /**
     * @brief _send 异步发送数据.
     *
     * @param content type:char*
     * @param length type:int
     * @param yield 协程
     *
     * @returns 0 for success else error code reference errcode.h.
     */
	int _send(const char *content, const int &length, boost::asio::yield_context yield);

    /**
     * @brief _receive 异步接收数据.
     *
     * @param size 接收数据大小.
     * @param yield 协程.
     *
     * @returns 0 for success else error code reference errcode.h.
     */
	int _receive(const int &size, boost::asio::yield_context yield);

    DataReceiveSignal data_send_signal_;/*<< 数据事件发生时发送的信号*/
};

typedef std::shared_ptr<TCPClient> TCPClientPtr;
#endif
