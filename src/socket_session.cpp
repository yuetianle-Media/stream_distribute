#include "stdafx.h"
#include "socket_session.h"

SocketSession::SocketSession(const string &remote_server, const int &port, const int &time_out)
    :work_ptr_(new boost::asio::io_service::work(io_svt_))
         , remote_addr( boost::asio::ip::address::from_string(remote_server), port)\
         , socket_ptr_(new boost::asio::ip::tcp::socket(io_svt_))\
         ,strand_ptr_(new boost::asio::strand(io_svt_))
{
}

SocketSession::~SocketSession()
{
}

int SocketSession::_connect()
{
    boost::system::error_code ec;
    if (NULL != socket_ptr_)
    {
        socket_ptr_->connect(remote_addr, ec);
    }
    else
    {
        std::cout << "socket is null!" << std::endl;
    }
    return 0;
}

int SocketSession::_async_connect()
{
    if (NULL != socket_ptr_)
    {
		socket_ptr_->async_connect(remote_addr, boost::bind(&SocketSession::connect_handler, shared_from_this(), boost::asio::placeholders::error));
        //boost::asio::async_connect(*socket_ptr_, remote_addr, boost::bind(&SocketSession::connect_handler, /*shared_from_this()*/this, boost::asio::placeholders::error));
        //socket_ptr_->async_connect(remote_addr, boost::bind(&SocketSession::connect_handler, shared_from_this()));
    }
    else
    {
        std::cout << "socket is null!" << std::endl;
    }
	return 0;
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
