/* Copyright(C)
 * For free
 * All right reserved
 *
 */

#include "stdafx.h"
#include "udpclient.h"

UDPClient::UDPClient(const int &local_port, const int &timeout_ms, const string &remote_addr, const int &remote_port)
    :UDPSocketSession(local_port, timeout_ms)
{

}

UDPClient::~UDPClient()
{

}

int UDPClient::write(char *data, const int &data_len)
{
	auto self = shared_from_this();
	if (data_len < segment_size_) 
	{
		boost::asio::spawn(strand_, [self, this, data, data_len](boost::asio::yield_context yield)
		{
			boost::system::error_code ec;
			socket_.async_send(boost::asio::buffer(data, data_len), yield[ec]);
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
				socket_.async_send(boost::asio::buffer(data + index, data_len), yield[ec]);
				if (ec)
				{
					break;
				}
			}
		});
	}
	return E_OK;
}

int UDPClient::receive(const int &data_len/*=0*/)
{
	return 0;
}

int UDPClient::connect()
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
	if (socket_.is_open())
	{

	}
	return result;
}

int UDPClient::_async_connect(boost::asio::yield_context yield)
{
	boost::system::error_code ec;
	socket_.async_connect(remote_ep_, yield[ec]);
	if (ec)
	{
		_close();
		return E_CONN_TIMEOUT;
	}
	return E_OK;
}

