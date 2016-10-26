#include "stdafx.h"
#include "tcpclient.h"

TCPClient::TCPClient(const string &remote_server, const int &port, const int &time_out/*ms*/)
	:SocketSession(remote_server, port, time_out)
{

}

TCPClient::~TCPClient()
{

}

int TCPClient::send(const char *content, const int &length)
{
	boost::system::error_code ec;
	boost::asio::write(socket_, boost::asio::buffer(content, length));
	if (ec)
	{
		cout << "send fail ip:" << remote_addr_.address().to_string()\
			<< "port:" << remote_addr_.port() << "size:" << length << std::endl;
		_close();
		return E_SEND_ERROR;
	}
	return E_OK;
}


int TCPClient::receive()
{
	boost::aligned_storage<0x8000> buff;
	boost::system::error_code ec;
	size_t sz = socket_.read_some(boost::asio::buffer((char*)buff.address(), buff.size), ec);
	while (!ec)
	{
		std::string tmp((char*)buff.address(), sz);
		std::cout << tmp << std::endl;
	}
	return 0;
}

int TCPClient::async_send(const char *content, const int &length)
{
	//vvlog_i("send cmd start");
	//if (!socket_.is_open())
	//{
	//	vvlog_w("connect is error reconnectiong");
	//	connect();
	//}
	boost::system::error_code ec;
	if (timer_ptr_)
	{
		timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
	}
	auto self = shared_from_this();
	int result = _run_sync_action([this, self, content, length](const coro_promise_ptr &porm\
		, const coro_timer_ptr &ptimer, boost::asio::yield_context yield)
	{

		try
		{
			boost::system::error_code ec;
			int result = _send(content, length, yield[ec]);
			if (nullptr != ptimer)
			{
				ptimer->cancel();
			}
			if (nullptr != porm)
			{
				porm->set_value(result);
			}
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << endl;

		}
		catch (std::error_code &e)
		{
			std::cout << e.value() << endl;
		}
	},timeout_);
	//_run_sync_action([]()
	//{}, timeout_);
	//boost::asio::async_write(socket_, boost::asio::buffer(content, length));
	if (result != E_OK)
	{
		//vvlog_e("send cmd end faile cmd:" << content << "errcode:" << result);
		//_close();
		return E_CONN_ERROR;
	}
	//else
	//{
	//	vvlog_i("send cmd end success content:" << content << "size:" << length);
	//}
	return E_OK;
}

int TCPClient::async_send_ext(const char *content, const int &length)
{
	auto self = shared_from_this();
	spawn(strand_, [this, self, content, length](boost::asio::yield_context yield)
	{
		try
		{
			boost::system::error_code ec;
			_send(content, length, yield[ec]);
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << endl;

		}
		catch (std::error_code &e)
		{
			std::cout << e.value() << endl;
		}
	});
	return E_OK;
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
		//if (timer_ptr_)
		//	timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
		boost::aligned_storage<0x8000> buff;
		for (;;)
		{
			//timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
			//socket_.async_receive()
			size_t sz = socket_.async_read_some(boost::asio::buffer((char*)buff.address(), buff.size), yield[ec]);
			//size_t sz = boost::asio::async_read(socket_, boost::asio::buffer((char*)buff.address(), buff.size), yield[ec]);
			if (!ec)
			{
				if (sz > 0)
				{
					string req((char*)buff.address(), sz);
					//std::cout << "receive data:" << req << std::endl;
					//std::cout << "start time:" << boost::local_time::cur
					//vvlog_i("receive len:" << sz << "len:" << req);
					try
					{
						if (!data_send_signal_.empty())
						{
							//vvlog_i("send data size:" << sz);
							std::cout << "receive data size:" << sz << " port:" << socket_.local_endpoint().port() << std::endl;
							data_send_signal_((char*)req.data(), sz);
							std::cout << "data:" << req.data();
							std::cout << "receive data end size:" << sz << " port:" << socket_.local_endpoint().port() << std::endl;
						}
						else
						{
							//vvlog_e("data signal slot empty");
						}
					}
					catch (std::exception &e)
					{
						//vvlog_e("data receive error" << e.what());
					}
				}
				else
				{
					std::cout << "no data receive" << std::endl;
				}
			}
			else
			{
				//vvlog_e("connect is brokent value:" << ec.value()\
					<< "mesg:" << ec.message() << "address:"\
					<< socket_.local_endpoint().address().to_string()\
					<< "port:" << socket_.local_endpoint().port());
				data_send_signal_(nullptr, E_MESG_END);
				_close();
				break;
				//std::cout << "err:" << ec.value() << "mesg:" << ec.message() << std::endl;
			}
		}

	});
	//_spawn_handle_timeout(timer_ptr_, nullptr);
	return 0;
}

int TCPClient::_send(const char * content, const int & length, boost::asio::yield_context yield)
{
	boost::system::error_code ec;
	if (!socket_.is_open())
	{
		return E_CONN_ERROR;
	}
	if (timer_ptr_)
	{
		timer_ptr_->expires_from_now(std::chrono::milliseconds(timeout_), ec);
	}
	boost::asio::async_write(socket_, boost::asio::buffer(content, length), yield[ec]);
	if (ec)
	{
		//vvlog_e("send fail error msg:" << ec.message());
		_close();
		return E_SEND_ERROR;
	}
	else
	{
		return E_OK;
	}
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

void TCPClient::unsubcribe_data_callback(boost::signals2::connection &subcriber)
{
	//vvlog_i("signal slot num:" << data_send_signal_.num_slots());
	data_send_signal_.disconnect_all_slots();
    //subcriber.disconnect();
}
boost::signals2::connection TCPClient::subcribe_data_callback(const DataReceiveSignal::slot_type &slot)
{
	return data_send_signal_.connect(slot);
}

