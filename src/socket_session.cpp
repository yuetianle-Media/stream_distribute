#include "stdafx.h"
#include "socket_session.h"
SocketSession::SocketSession(const string &remote_server, const int &port, const int &time_out)
    : io_svt_ptr_(make_shared<boost::asio::io_service>())\
    , socket_(*io_svt_ptr_)\
	, remote_addr_(boost::asio::ip::address::from_string(remote_server), port)\
	, strand_(socket_.get_io_service())\
	, timer_ptr_(new boost::asio::steady_timer(socket_.get_io_service()))\
	, timeout_(time_out)
{
	work_ptr_ = make_shared<boost::asio::io_service::work>(*io_svt_ptr_);
	auto svc = io_svt_ptr_;

	thread thrd([svc]()
	{
		boost::system::error_code ec;
		svc->run(ec);
#ifdef _DEBUG
		std::cout << "service terminated. ec:" << ec.value() << "msg:" << ec.message()<< std::endl;
#endif // _DEBUG
	});

	thrd.detach();
}

SocketSession::~SocketSession()
{
}

bool SocketSession::resize_recv_size(const long int &recv_size)
{
	boost::system::error_code ec;
	boost::asio::ip::tcp::socket::receive_buffer_size bz(0);
	socket_.get_option(bz, ec);
	if (!ec)
	{
		socket_.set_option(boost::asio::ip::tcp::socket::receive_buffer_size(recv_size),ec);
		if (!ec)
		{
			socket_.get_option(bz, ec);
			return true;
		}
	}
	return false;
}

bool SocketSession::resize_send_size(const long int &send_size)
{
	boost::system::error_code ec;
	boost::asio::ip::tcp::socket::send_buffer_size bz(0);
	socket_.get_option(bz, ec);
	if (!ec)
	{
		socket_.set_option(boost::asio::ip::tcp::socket::send_buffer_size(send_size), ec);
		if (!ec)
		{
			socket_.get_option(bz, ec);
			return true;
		}
	}
	return false;
}

int SocketSession::_connect_ex(const string & content)
{
	auto self = shared_from_this();
	int result = _run_sync_action([this, self, content](coro_promise_ptr prom, coro_timer_ptr ptimer, boost::asio::yield_context yield)
	{
		int result = _async_connect(content, yield);
		if (ptimer != nullptr)
		{
			ptimer->cancel();
		}
		if (prom != nullptr)
		{
			prom->set_value(result);
		}
	}, timeout_);

	if (E_OK == result)
	{
		//vvlog_i("connect success local address:" << socket_.local_endpoint().address().to_string()\
<< "port:" << socket_.local_endpoint().port());
	}
	else
	{
		//vvlog_i("connect fail local address:" << socket_.local_endpoint().address().to_string()\
<< "port:" << socket_.local_endpoint().port() << "err:" << result);
	}
	if (socket_.is_open())
	{
		resize_recv_size(0x1000);
		//resize_recv_buffer(m_recv_buff_size);
		//resize_send_buffer(m_send_buff_size);
		set_no_delay(true);
	}
	return result;
}

void SocketSession::_close()
{
	boost::system::error_code ec;
	if (socket_.is_open())
	{
		socket_.close(ec);
	}
	if (timer_ptr_)
	{
		timer_ptr_->cancel(ec);
	}
}

int SocketSession::_async_connect(const string &content, boost::asio::yield_context yield)
{
	boost::system::error_code ec;
	socket_.async_connect(remote_addr_, yield[ec]);
	if (ec)
	{
		_close();
		return E_CONN_ERROR;
	}
	if (0 < content.length())
	{
		boost::asio::async_write(socket_, boost::asio::buffer(content.data()\
			, content.length())\
			, [this](boost::system::error_code ec, int size)
		{
			if (ec)
			{
				_close();
			}
		});
	}
	return E_OK;
}

int SocketSession::connect()
{
	int ec = _connect_ex("");
	return ec;
}

int SocketSession::connect_ex(const string & content)
{
	int ec = _connect_ex(content);
	return ec;
}

bool SocketSession::set_no_delay(bool value)
{
	boost::system::error_code ec;
	socket_.set_option(boost::asio::ip::tcp::no_delay(value), ec);
	if (!ec)
	{
		//cout << "Set Socket No Delay To " << val << " OK" << endl;
		return true;
	}
	return false;
}

int SocketSession::_run_sync_action(coro_action operation_action, const int & time_out)
{
	auto prom = make_shared<coro_promise>();
	coro_timer_ptr timer(new coro_timer(*io_svt_ptr_));
	boost::system::error_code ec;
	auto self = shared_from_this();
	timer->expires_from_now(std::chrono::milliseconds(time_out), ec);
	boost::asio::spawn(strand_, [this, self, prom, timer](boost::asio::yield_context yield)
	{
		boost::system::error_code ec;
		timer->async_wait(yield[ec]);
		if (ec)
		{
			return;
		}
		else if (timer->expires_from_now() <= std::chrono::milliseconds(0))
		{
			try
			{
				//vvlog_w("time out");
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

void SocketSession::_spawn_handle_timeout(const coro_timer_ptr & ptimer, const coro_promise_ptr & prom)
{
	auto self = shared_from_this();
	boost::asio::spawn(strand_,
		[this, self, ptimer, prom](boost::asio::yield_context yield)
	{
		while (socket_.is_open())
		{
			boost::system::error_code ec;
			if (nullptr != ptimer)
			{
				ptimer->async_wait(yield[ec]);

				if (ptimer->expires_from_now() <= chrono::milliseconds(0))
				{
					//socket_.close(ec);
					ptimer->cancel();
					if (prom != nullptr)
					{
						if (prom->get_future().valid())
						{
							prom->set_value(E_CONN_TIMEOUT);
						}
					}
				}
			}
		}
	});
}
