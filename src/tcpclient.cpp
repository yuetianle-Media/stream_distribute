
#include "tcpclient.h"

TCPClient::TCPClient(const string &remote_server, const int &port, const int &time_out/*s*/)
{

}

TCPClient::~TCPClient()
{

}

int TCPClient::send(const char *content, const int &length)
{
	boost::system::error_code ec;
	if (timer_ptr_)
	{
		timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
	}
	boost::asio::write(socket_, boost::asio::buffer(content, length));
	if (ec)
	{
		_close();
	}
	else
	{
		cout << "send size:" << length << std::endl;
	}

}


int TCPClient::receive()
{

}

int TCPClient::async_send(const char *content, const int &length)
{
	boost::system::error_code ec;
	if (timer_ptr_)
	{
		timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
	}
	boost::asio::async_write(socket_, boost::asio::buffer(content, length), yield[ec]);
	if (ec)
	{
		_close();
	}
	else
	{
		cout << "send size:" << length << std::endl;
	}

}

int TCPClient::async_receive()
{
	return 0;

}

int TCPClient::wait_response()
{
	if (socket_.is_open())
	{
		set_no_delay(true);
	}
	auto self = shared_from_this();
	boost::asio::spawn(strand_,
		[this, self](boost::asio::yield_context yield)
	{
		boost::system::error_code ec;
		m_timer->expires_from_now(std::chrono::milliseconds(m_timeout_ms), ec);
		boost::aligned_storage<0x8000> buff;
		for (;;)
		{
			m_timer->expires_from_now(std::chrono::milliseconds(m_timeout_ms), ec);
			size_t sz = m_socket.async_read_some(boost::asio::buffer((char*)buff.address(), buff.size), yield[ec]);
			if (!ec)
			{
				if (sz > 0)
				{
					string req((char*)buff.address(), sz);
					//将接收到的数据发送出去
					//_handle_request(move(req), m_timer, yield);
				}
			}
			else
			{
				_close();
				break;
			}
		}

	});
	_spawn_handle_timeout(m_timer, nullptr);
}

int TCPClient::_send(const char * content, const int & length, boost::asio::yield_context yield)
{
	boost::system::error_code ec;
	if (timer_ptr_)
	{
		timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
	}
	boost::asio::write(socket_, boost::asio::buffer(content, length), yield[ec]);
	if (ec)
	{
		_close();
	}
	else
	{
		cout << "send size:" << length << std::endl;
	}
	return 0;
}

int TCPClient::_receive(const int & size)
{
	if (0 < size)
	{
		auto fz = make_shared<int64_t>(file_size);
		boost::system::error_code ec;
		boost::aligned_storage<0x8000> buff;
		while (*fz > 0)
		{
			ec = socket_.async_read_some(boost::asio::buffer(buff.address(), buff.size), yield[ec]);
			if (!ec)
			{
				if (timer_ptr_)
				{
					timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
				}
				//发送接收数据signal
			}
			else
			{
				cout << "read fail mesg:" << ec.message() << std::endl;
				_close();
			}

		}
	}
	return 0;
}

