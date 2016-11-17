#include "stdafx.h"
#include "udp_socket_session.h"

shared_ptr<boost::asio::io_service> UDPSocketSession::_s_shared_service(new boost::asio::io_service);

shared_ptr<boost::asio::io_service::work> UDPSocketSession::_s_shared_service_work;

std::mutex UDPSocketSession::_s_mtx_init;


void UDPSocketSession::s_init_thread_pool(const int &pool_size)
{
	if (!_s_shared_service_work)
	{
		v_lock(lk, _s_mtx_init);
		if (!_s_shared_service_work)
		{
			_s_shared_service_work = std::make_shared<boost::asio::io_service::work>(*_s_shared_service);
			int size = pool_size <= 0 ? std::thread::hardware_concurrency() : pool_size;
			auto svc = _s_shared_service;
			for (int i = 0; i < size; ++i)
			{
				std::thread task([svc]() 
				{
					boost::system::error_code ec;
					svc->run(ec);
					std::cout << "s_shared_service complete." << std::endl;
				});
				task.detach();
			}
		}
	}
}


void UDPSocketSession::s_clean_thread_pool()
{
	if (_s_shared_service_work)
	{
		v_lock(lk, _s_mtx_init);
		if (_s_shared_service_work)
		{
			_s_shared_service_work.reset();
		}
	}
}

UDPSocketSession::UDPSocketSession(const int &local_port, const int &timeout_ms, const std::string &local_ip/*="127.0.0.1"*/)
    : local_port_(local_port)\
	, time_out_ms_(timeout_ms)\
	, use_shared_service_(false)
	, io_svt_ptr_(use_shared_service_?_s_shared_service:std::make_shared<boost::asio::io_service>())
	, udp_socket_(*io_svt_ptr_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), local_port))
	, strand_(udp_socket_.get_io_service())\
	, timer_ptr_(0)
{
	work_ = make_shared<boost::asio::io_service::work>(*io_svt_ptr_);
	boost::asio::ip::address_v4 local_interface = boost::asio::ip::address_v4::from_string(local_ip);
	if (!local_interface.is_loopback())
	{
		boost::asio::ip::multicast::outbound_interface bound_interface(local_interface);
		boost::system::error_code ec;
		udp_socket_.set_option(bound_interface, ec);
		if (!ec)
		{
			std::cout << "bound success" << std::endl;
		}
	}
	auto svc = io_svt_ptr_;
	if (use_shared_service_)
	{
		s_init_thread_pool(4);
	}
	else
	{
		thread thrd([svc]
		{
			boost::system::error_code ec;
			svc->run(ec);
		});
		thrd.detach();
	}
}

UDPSocketSession::~UDPSocketSession()
{

}


bool UDPSocketSession::resize_send_buffer_size(const long int &send_size)
{
	boost::system::error_code ec;
	boost::asio::ip::tcp::socket::send_buffer_size bz(0);
	udp_socket_.get_option(bz, ec);
	if (!ec)
	{
		udp_socket_.set_option(boost::asio::ip::tcp::socket::send_buffer_size(send_size), ec);
		if (!ec)
		{
			udp_socket_.get_option(bz, ec);
			return true;
		}
	}
	return false;

}


bool UDPSocketSession::resize_receive_buffer_size(const long int &receive_size)
{
	return true;
}


bool UDPSocketSession::set_reuse(const bool &is_reuse)
{
	boost::asio::socket_base::reuse_address reuse(is_reuse);
	boost::system::error_code ec;
	udp_socket_.get_option(reuse, ec);
	if (!ec)
	{
		reuse = is_reuse;
		udp_socket_.set_option(reuse, ec);
		if (!ec)
		{
			udp_socket_.get_option(reuse, ec);
			return E_OK;
		}
	}
	return E_PARAM_ERROR;
}


bool UDPSocketSession::set_debug(const bool &is_debug)
{
	boost::asio::socket_base::debug option(is_debug);
	boost::system::error_code ec;
	udp_socket_.get_option(option, ec);
	if (!ec)
	{
		option = is_debug;
		udp_socket_.set_option(option, ec);
		if (!ec)
		{
			udp_socket_.get_option(option, ec);
			return E_OK;
		}
	}
}


bool UDPSocketSession::set_noblock(const bool &is_block)
{

	boost::system::error_code ec;
	bool blocking = udp_socket_.non_blocking();
	if (blocking != is_block)
	{
		udp_socket_.non_blocking(is_block, ec);
		if (!ec)
		{
			blocking = udp_socket_.non_blocking();
			return true;
		}
	}
	return false;
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
