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
	static std::shared_ptr<boost::asio::io_service> _s_shared_service;
	static std::shared_ptr<boost::asio::io_service::work> _s_shared_service_work;
	static std::mutex _s_mtx_init;

	static void s_init_thread_pool(const int &pool_size);
	static void s_clean_thread_pool();
public:
	UDPSocketSession(const int &local_port, const int &timeout_ms, const std::string &local_ip="127.0.0.1");
	~UDPSocketSession();

	typedef std::promise<int> coro_promise;
	typedef std::shared_ptr<coro_promise> coro_promise_ptr;
	typedef boost::asio::steady_timer coro_timer;
	typedef std::shared_ptr<coro_timer> coro_timer_ptr;

	boost::asio::ip::udp::socket& socket() { return udp_socket_; }
	boost::asio::io_service::strand& strand() { return strand_; }
	bool resize_send_buffer_size(const long int &send_size);
	bool resize_receive_buffer_size(const long int &receive_size);
	bool set_reuse(const bool &is_reuse);
	bool set_debug(const bool &is_debug);
	bool set_noblock(const bool &is_block);
protected:
	int local_port_;
	int time_out_ms_;
	bool use_shared_service_;
	std::shared_ptr<boost::asio::io_service> io_svt_ptr_;
	boost::asio::ip::udp::socket udp_socket_;
	std::shared_ptr<boost::asio::io_service::work> work_;
	boost::asio::io_service::strand strand_;

	coro_timer_ptr timer_ptr_;
	int _close();
	typedef std::function<void(const coro_promise_ptr &prom, const coro_timer_ptr &timer, boost::asio::yield_context)> coro_action;
	int _run_sync_action(coro_action operation_action, const long int &time_out_mcro);
};

typedef std::shared_ptr<UDPSocketSession> UDPSocketSessionPtr;
