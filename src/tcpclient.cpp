
#include "stdafx.h"
#include "tcpclient.h"

TCPClient::TCPClient(const string &remote_server, const int &port, const int &time_out/*s*/)
	:SocketSession(remote_server, port, time_out)
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
	return 0;
}


int TCPClient::receive()
{
	return 0;
}

int TCPClient::async_send(const char *content, const int &length)
{
	boost::system::error_code ec;
	if (timer_ptr_)
	{
		timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
	}
	//boost::asio::async_write(socket_, boost::asio::buffer(content, length));
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
		timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
		boost::aligned_storage<0x8000> buff;
		for (;;)
		{
			timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
			size_t sz = socket_.async_read_some(boost::asio::buffer((char*)buff.address(), buff.size), yield[ec]);
			if (!ec)
			{
				if (sz > 0)
				{
					string req((char*)buff.address(), sz);
                    data_send_signal_((char*)buff.address(), sz);
				}
			}
			else
			{
				_close();
				break;
			}
		}

	});
	_spawn_handle_timeout(timer_ptr_, nullptr);
	return 0;
}

int TCPClient::_send(const char * content, const int & length, boost::asio::yield_context yield)
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
	return 0;
}

int TCPClient::_receive(const int & size, boost::asio::yield_context yield)
{
	if (0 < size)
	{
		auto fz = std::make_shared<int64_t>(size);
		boost::system::error_code ec;
		boost::aligned_storage<0x8000> buff;
		while (*fz > 0)
		{
			int read_size = socket_.async_read_some(boost::asio::buffer(buff.address(), buff.size), yield[ec]);
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

void TCPClient::unsubcribe_data_callback(boost::signals2::connection subcriber)
{
    subcriber.disconnect();
}
boost::signals2::connection TCPClient::subcribe_data_callback(const DataReceiveSignal::slot_type &slot)
{
	return data_send_signal_.connect(slot);
}

