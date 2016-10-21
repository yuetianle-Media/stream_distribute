#include "stdafx.h"
#include "udp_socket_session.h"

UDPSocketSession::UDPSocketSession(const int &local_port, const int &timeout_ms)
    : local_port_(local_port)\
	, time_out_ms_(timeout_ms)\
	, io_svt_ptr_(make_shared<boost::asio::io_service>())\
	, udp_socket_(*io_svt_ptr_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), local_port))
	, strand_(udp_socket_.get_io_service())\
	, timer_ptr_(0)
{
	work_ = make_shared<boost::asio::io_service::work>(*io_svt_ptr_);
	auto svc = io_svt_ptr_;
	thread thrd([svc]
	{
		boost::system::error_code ec;
		svc->run(ec);
	});
	thrd.detach();

}

UDPSocketSession::~UDPSocketSession()
{

}

int UDPSocketSession::_close()
{
	boost::system::error_code ec;
	if (udp_socket_.is_open())
	{
		udp_socket_.close(ec);
	}
	if (timer_ptr_)
	{
		timer_ptr_->cancel(ec);
	}
	return E_OK;
	return 0;
}

int UDPSocketSession::_run_sync_action(coro_action operation_action, const long int &time_out_mcro)
{
	auto prom = make_shared<coro_promise>();
	coro_timer_ptr timer(new coro_timer(*io_svt_ptr_));
	boost::system::error_code ec;
	auto self = shared_from_this();
	timer->expires_from_now(std::chrono::microseconds(time_out_mcro), ec);
	boost::asio::spawn(strand_, [this, self, prom, timer](boost::asio::yield_context yield) 
	{
		boost::system::error_code ec;
		timer->async_wait(yield[ec]);
		if (ec)
		{
			return;
		}
		else if (timer->expires_from_now() <= std::chrono::microseconds(0))
		{
			try 
			{
				if (nullptr != prom)
				{
					prom->set_value(E_CONN_TIMEOUT);
				}
			}
			catch (std::future_error & fe)
			{
				cout << fe.what() << endl;
			}
			catch (exception& e)
			{
				cout << e.what() << endl;
			}
		}
	});

	boost::asio::spawn(strand_, bind(operation_action, prom, timer, placeholders::_1));
	int result = prom->get_future().get();
	return result;
}
