#include "stdafx.h"
#include "socket_session.h"
SocketSession::SocketSession(const string &remote_server, const int &port, const int &time_out)
    : io_svt_ptr_(make_shared<boost::asio::io_service>())\
	, remote_addr_(boost::asio::ip::address::from_string(remote_server), port)\
	, socket_(*io_svt_ptr_)\
	, strand_(socket_.get_io_service())\
	, timeout_(time_out)
	, timer_ptr_(0)
{
	//timer_ptr_.reset();
	//work_ptr_ = boost::make_shared<boost::asio::io_service::work>(boost::ref(*io_svt_ptr_));
	work_ptr_ = make_shared<boost::asio::io_service::work>(*io_svt_ptr_);

}

SocketSession::~SocketSession()
{
}


int SocketSession::_connect()
{
    boost::system::error_code ec;
	socket_.connect(remote_addr_, ec);
	if (ec)
	{
		std::cout << "socket is null!" << std::endl;
		return -1;
	}
	else
		return 0;
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
	cout << "connect = " << (result == 0) << endl;
	if (socket_.is_open())
	{
		//resize_recv_buffer(m_recv_buff_size);
		//resize_send_buffer(m_send_buff_size);
		//set_no_delay(true);
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
		//boost::asio::async_write(socket_, boost::asio::buffer(content.data(), content.length()), boost::bind(&SocketSession::write_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
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
			ptimer->async_wait(yield[ec]);
			
			if (ptimer->expires_from_now() <= chrono::milliseconds(0))
			{
				socket_.close(ec);
				if (prom != nullptr)
				{
					if (prom->get_future().valid())
					{
						prom->set_value(E_CONN_TIMEOUT);
					}
				}
			}
		}
	});

}

void SocketSession::write_handler(const boost::system::error_code & ec, const int & size)
{
	if (ec)
	{
		_close();
	}
}

void SocketSession::connect_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::cout << "connect the server faile!" << std::endl;
    }
    else
    {
        std::cout << "connect success.";
    }
}
