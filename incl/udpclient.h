/**
 * @file udpclient.h
 * @brief UDPClient for multicast
 * @author lee, shida23577@hotmail.com
 * @version 1.0.0
 * @date 2016-09-14
 */

#pragma once
#include "udp_socket_session.h"
#include <string>

class UDPClient:public UDPSocketSession
{
public:
	UDPClient(const int &local_port, const int &timeout_ms, const string &remote_addr, const int &remote_port);
	~UDPClient();


	int write(char *data, const int &data_len);
	int receive(const int &data_len=0);
	int connect();
protected:
	int _async_connect(boost::asio::yield_context yield);
private:
	boost::asio::ip::udp::endpoint remote_ep_;
	int segment_size_;
};

typedef std::shared_ptr<UDPClient> UDPClientPtr;


