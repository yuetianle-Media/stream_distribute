/* Copyright(C)
 * For free
 * All right reserved
 *
 */

#include "stdafx.h"
#include "udpclient.h"

UDPClient::UDPClient(const int &local_port, const int &timeout_ms, const string &remote_addr, const int &remote_port)
    :UDPSocketSession(local_port, timeout_ms), remote_ep_(boost::asio::ip::address::from_string(remote_addr), remote_port)
{

}

UDPClient::~UDPClient()
{

}

int UDPClient::write(char *data, const int &data_len)
{
	auto self = shared_from_this();
	//auto write_coro_timer = make_shared<coro_timer>(*io_svt_ptr_);
	if (data_len < segment_size_) 
	{
		//boost::asio::spawn(strand_, [self, this, write_coro_timer, data, data_len](boost::asio::yield_context yield)
		//{
		//	boost::system::error_code ec;
		//	write_coro_timer->async_wait(yield[ec]);
		//	if (ec)
		//	{
		//		//std::chrono::steady_clock::duration time_reamin = write_coro_timer->expires_from_now();
		//		//this_thread::sleep_for(std::chrono::milliseconds(time_reamin));
		//		return ec;
		//	}
		//	else
		//	{
		//		if (write_coro_timer->expires_from_now() < std::chrono::milliseconds(0))
		//		{
		//		}
		//	}
		//});
		boost::asio::spawn(strand_, [self, this, data, data_len](boost::asio::yield_context yield)
		{
			boost::system::error_code ec;
			udp_socket_.async_send(boost::asio::buffer(data, data_len), yield[ec]);
		});
	}
	else
	{
		boost::asio::spawn(strand_, [self, this, data, data_len](boost::asio::yield_context yield)
		{
			boost::system::error_code ec;
			for (int index = 0; index < data_len; index += segment_size_)
			{
				int remain_size = data_len - index;
				int send_size = remain_size > segment_size_ ? segment_size_ : remain_size;
				udp_socket_.async_send(boost::asio::buffer(data + index, data_len), yield[ec]);
				if (ec)
				{
					break;
				}
			}
		});
	}
	return E_OK;
}

int UDPClient::write(char *data, const int &data_len, const double &need_time)
{
	auto self = shared_from_this();
	auto write_coro_timer = make_shared<coro_timer>(*io_svt_ptr_);
	if (data_len < segment_size_) 
	{
		boost::asio::spawn(strand_, [self, this, write_coro_timer, data, data_len](boost::asio::yield_context yield)
		{
			boost::system::error_code ec;
			write_coro_timer->async_wait(yield[ec]);
			if (ec)
			{
				//std::chrono::steady_clock::duration time_reamin = write_coro_timer->expires_from_now();
				//this_thread::sleep_for(std::chrono::milliseconds(time_reamin));
				return ec;
			}
			else
			{
				if (write_coro_timer->expires_from_now() < std::chrono::milliseconds(0))
				{
				}
			}
		});
		boost::asio::spawn(strand_, [self, this, data, data_len](boost::asio::yield_context yield)
		{
			boost::system::error_code ec;
			udp_socket_.async_send(boost::asio::buffer(data, data_len), yield[ec]);
		});
	}
	else
	{
		boost::asio::spawn(strand_, [self, this, data, data_len](boost::asio::yield_context yield)
		{
			boost::system::error_code ec;
			for (int index = 0; index < data_len; index += segment_size_)
			{
				int remain_size = data_len - index;
				int send_size = remain_size > segment_size_ ? segment_size_ : remain_size;
				udp_socket_.async_send(boost::asio::buffer(data + index, data_len), yield[ec]);
				if (ec)
				{
					break;
				}
			}
		});
	}
	
	this_thread::sleep_for(chrono::microseconds(static_cast<int>(need_time)));
	return E_OK;

}

int UDPClient::receive(const int &data_len/*=0*/)
{
	return 0;
}

int UDPClient::connect()
{
	return _connect();
}

int UDPClient::_write(char * data, const int & data_len)
{
	auto self = shared_from_this();
	auto write_coro_timer = make_shared<coro_timer>(*io_svt_ptr_);
	if (data_len < segment_size_) 
	{
		boost::asio::spawn(strand_, [self, this, write_coro_timer, data, data_len](boost::asio::yield_context yield)
		{
			boost::system::error_code ec;
			write_coro_timer->async_wait(yield[ec]);
			if (ec)
			{
				//std::chrono::steady_clock::duration time_reamin = write_coro_timer->expires_from_now();
				//this_thread::sleep_for(std::chrono::milliseconds(time_reamin));
				return ec;
			}
			else
			{
				if (write_coro_timer->expires_from_now() < std::chrono::milliseconds(0))
				{
				}
			}
		});
		boost::asio::spawn(strand_, [self, this, data, data_len](boost::asio::yield_context yield)
		{
			boost::system::error_code ec;
			udp_socket_.async_send(boost::asio::buffer(data, data_len), yield[ec]);
		});
	}
	else
	{
		boost::asio::spawn(strand_, [self, this, data, data_len](boost::asio::yield_context yield)
		{
			boost::system::error_code ec;
			for (int index = 0; index < data_len; index += segment_size_)
			{
				int remain_size = data_len - index;
				int send_size = remain_size > segment_size_ ? segment_size_ : remain_size;
				udp_socket_.async_send(boost::asio::buffer(data + index, data_len), yield[ec]);
				if (ec)
				{
					break;
				}
			}
		});
	}
	return E_OK;
}

int UDPClient::_connect()
{
	boost::system::error_code ec;
	auto self = shared_from_this();
	int result = _run_sync_action([this, self](coro_promise_ptr prom, coro_timer_ptr timer, boost::asio::yield_context yield)
	{
		int result = _async_connect(yield);
		if (nullptr != timer)
		{
			timer->cancel();
		}
		if (nullptr != prom)
		{
			prom->set_value(result);
		}
	}, time_out_ms_);
	cout << "connect = " << (result == 0) << std::endl;
	if (udp_socket_.is_open())
	{

	}
	return result;
}

int UDPClient::_async_connect(boost::asio::yield_context yield)
{
	boost::system::error_code ec;
	udp_socket_.async_connect(remote_ep_, yield[ec]);
	if (ec)
	{
		_close();
		return E_CONN_TIMEOUT;
	}
	return E_OK;
}

