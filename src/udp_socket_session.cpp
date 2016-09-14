#include "stdafx.h"
#include "udp_socket_session.h"

UDPSocketSession::UDPSocketSession(const int &local_port, const int &timeout_ms)
    : io_svt_(make_shared<boost::asio::io_service>())\
          , local_port_(local_port)\
          , time_out_ms_(timeout_ms)
          , work_(make_shared<boost::asio::io_service::work>(*io_svt_)\
          ,  socket_(*io_svt_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), local_port_))\
          , strand_(socket_.get_io_service()))
{

}
