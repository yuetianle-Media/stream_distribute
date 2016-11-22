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
	UDPClient(const int &local_port, const int &timeout_ms, const std::string &local_ip="127.0.0.1");
	~UDPClient();

	int sync_write(char *data, const long &data_size, const string &ip, const long &port);
	int write_ext(char *data, const long int &data_len\
		, const string &remote_addr, const int &remote_port);
	int write(char *data, const int &data_len);
	int write(char *data, const int &data_len, const double &need_time);
	int receive(const int &data_len=0);
	int connect();
	string remote_ip() const { return remote_ep_.address().to_string(); }
	int remote_port() const { return remote_ep_.port(); }
private:
	int _write(char *data, const int &data_len);
	int _connect();
	int _async_connect(boost::asio::yield_context yield);
	boost::asio::ip::udp::endpoint remote_ep_;
	int segment_size_;
};

typedef std::shared_ptr<UDPClient> UDPClientPtr;


