/**
 * @file socket_session.h
 * @brief tcp base socket.
 * @author lee, shida23577@hotmail.com
 * @version 1.0.1
 * @date 2016-09-27
 */

#ifndef _SOCKET_SESSION_H_
#define _SOCKET_SESSION_H_

#pragma once

#include "pre_std_basic.h"
#include "pre_boost_basic.h"
#include "errcode.h"

using namespace std;

/**
 * @brief base socket wrapper for tcp session.
 */
class SocketSession: public std::enable_shared_from_this<SocketSession>
{
public:
    SocketSession(const string &remote_server, const int &port, const int &time_out/*ms*/);
    ~SocketSession();

public:
    typedef std::promise<int> coro_promise;
    typedef std::shared_ptr<coro_promise> coro_promise_ptr;
    typedef boost::asio::steady_timer coro_timer;
    typedef std::shared_ptr<coro_timer> coro_timer_ptr;
    typedef boost::asio::handler_type<boost::asio::yield_context, void(int)>::type async_result_handler;

	boost::asio::ip::tcp::socket& socket() { return socket_; }
	bool resize_recv_size(const long int &recv_size);
	bool resize_send_size(const long int &send_size);
    /**
     * @brief send 同步发送数据.
     *
     * @param content type:char*
     * @param length type:int
     *
     * @returns 0 for success else error code reference errcode.h.
     */
    virtual int send(const char *content, const int &length)=0;

    /**
     * @brief receive 同步接收数据.
     *
     * @returns 0 for success else error code reference errcode.h.
     */
    virtual int receive()=0;

    /**
     * @brief async_send 异步发送数据.
     *
     * @param content type:char*
     * @param length type:int
     *
     * @returns 0 for success else error code reference errcode.h.
     */
    virtual int async_send(const char *content, const int &length)=0;

    /**
     * @brief async_receive
     *
     * @returns
     */
    virtual int async_receive()=0;

    /**
     * @brief async connect to the endpoint.
     *
     * @returns 0 for success else error code reference errcode.h.
     */
    virtual int connect();

    /**
     * @brief connect_ex async connect to the endpoint.
     *
     * @param content type:string 发送内容
     *
     * @returns 0 for success else error code reference errcode.h.
     */
    virtual int connect_ex(const string &content);


    /**
     * @brief set_no_delay 设置socket 不延迟选项
     *
     * @param value
     *
     * @returns
     */
    bool set_no_delay(bool value);

protected:

    std::shared_ptr<boost::asio::io_service> io_svt_ptr_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::endpoint remote_addr_;

    std::shared_ptr<boost::asio::io_service::work> work_ptr_;/*<< 确保socket保活*/
    boost::asio::io_service::strand strand_;/*<< 确保线程同步*/
    std::shared_ptr<boost::asio::steady_timer> timer_ptr_;/*<< 超时timer*/
    int timeout_;

    /**
     * @brief _spawn_handle_timeout 超时处理.
     *
     * @param ptimer 超时timer.
     * @param prom 返回值.
     */
    virtual void _spawn_handle_timeout(const coro_timer_ptr& ptimer, const coro_promise_ptr& prom);

    /**
     * @brief _close 关闭socket连接.
     */
    virtual void _close();

    typedef std::function<void(const coro_promise_ptr &prom, const coro_timer_ptr &timer, boost::asio::yield_context)> coro_action;

    /**
     * @brief _run_sync_action 同步的形式进行异步操作(带超时)
     *
     * @param operation_action 异步操作
     * @param time_out type:int 超时时间
     *
     * @returns 0 for success else error code reference errcode.h.
     */
    int _run_sync_action(coro_action operation_action, const int &time_out);
private:

    /**
     * @brief _connect_ex tcp 连接.
     *
     * @param content type:string 发送内容.
     *
     * @returns 0 for success else return error code.
     */
    int _connect_ex(const string &content);

    /**
     * @brief _async_connect 异步连接
     *
     * @param content 发送内容.
     * @param yield 协程对象.
     *
     * @returns 0 for success else return error code.
     */
    int _async_connect(const string &content, boost::asio::yield_context yield);
};

typedef boost::shared_ptr<SocketSession> session_ptr;

#endif
