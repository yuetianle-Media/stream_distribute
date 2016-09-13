#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/asio/spawn.hpp>
//#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
//#include <boost/make_shared.hpp>
//#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "pre_std_basic.h"

#include "errcode.h"
using namespace std;

class SocketSession: public std::enable_shared_from_this<SocketSession>
{
    public:
        SocketSession(const string &remote_server, const int &port, const int &time_out/*s*/);
        ~SocketSession();

		typedef std::promise<int> coro_promise;
		typedef std::shared_ptr<coro_promise> coro_promise_ptr;
		typedef boost::asio::steady_timer coro_timer;
		typedef std::shared_ptr<coro_timer> coro_timer_ptr;

		typedef boost::asio::handler_type<boost::asio::yield_context, void(int)>::type async_result_handler;

        virtual int send(const char *content, const int &length)=0;
        virtual int receive()=0;

        virtual int async_send(const char *content, const int &length)=0;
        virtual int async_receive()=0;

        //void handle_write(const boost::system::error_code &ec, size_t byte_transferred);
        //void handle_read(const boost::system::error_code &ec);
		virtual int connect();
		virtual int connect_ex(const string &content);
		bool set_no_delay(bool value);
	protected:
		//typedef boost::function<void(const coro_promise_ptr &prom, const coro_timer_ptr &timer, boost::asio::yield_context)> coro_action;
		typedef function<void(const coro_promise_ptr &prom, const coro_timer_ptr &timer, boost::asio::yield_context)> coro_action;
		int _run_sync_action(coro_action operation_action, const int &time_out);
        std::shared_ptr<boost::asio::io_service> io_svt_ptr_;
        //boost::shared_ptr<boost::asio::io_service> io_svt_ptr_;
        std::shared_ptr<boost::asio::io_service::work> work_ptr_;
        //boost::shared_ptr<boost::asio::io_service::work> work_ptr_;
        boost::asio::strand strand_;

        boost::asio::ip::tcp::endpoint remote_addr_;
        boost::asio::ip::tcp::socket socket_;
        //boost::shared_ptr<boost::asio::steady_timer> timer_ptr_;
        std::shared_ptr<boost::asio::steady_timer> timer_ptr_;
		virtual void _spawn_handle_timeout(const coro_timer_ptr& ptimer, const coro_promise_ptr& prom);
		int timeout_;

    private:

		void write_handler(const boost::system::error_code &ec, const int &size);
        void connect_handler(const boost::system::error_code &ec);


        int _connect();
		int _connect_ex(const string &content);
        int _async_connect(const string &content, boost::asio::yield_context yield);

		virtual void _close();


};

//typedef boost::shared_ptr<SocketSession> session_ptr;
typedef boost::shared_ptr<SocketSession> session_ptr;
