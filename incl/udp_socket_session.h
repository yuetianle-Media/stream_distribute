 /**
 * @file udp_socket_session.h
 * @brief udp socket class
 * @author lee, shida23577@hotmail.com
 * @version 1.0.0
 * @date 2016-09-14
 */
#pragma once
#include "errcode.h"
#include "pre_boost_basic.h"
#include "pre_std_basic.h"
using namespace std;


class UDPSocketSession :public std::enable_shared_from_this<UDPSocketSession>
{
public:
	UDPSocketSession(const int &local_port, const int &timeout_ms);
	~UDPSocketSession();

	typedef std::promise<int> coro_promise;
	typedef std::shared_ptr<coro_promise> coro_promise_ptr;
	typedef boost::asio::steady_timer coro_timer;
	typedef std::shared_ptr<coro_timer> coro_timer_ptr;
protected:
	boost::asio::ip::udp::socket socket_;
	int local_port_;
	int time_out_ms_;
	shared_ptr<boost::asio::io_service> io_svt_;
	shared_ptr<boost::asio::io_service::work> work_;
	boost::asio::io_service::strand strand_;

	coro_timer_ptr timer_ptr_;
	int _close();
	typedef std::function<void(const coro_promise_ptr &prom, const coro_timer_ptr &timer, boost::asio::yield_context)> coro_action;
	int _run_sync_action(coro_action operation_action, const int &time_out);
};

typedef std::shared_ptr<UDPSocketSession> UDPSocketSessionPtr;
