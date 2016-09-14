/**
 * @file udpclient.h
 * @brief UDPClient for multicast
 * @author lee, shida23577@hotmail.com
 * @version 1.0.0
 * @date 2016-09-14
 */

#include "udp_socket_session.h"

class UDPClient:public UDPSocketSession
{
    public:
        UDPClient(const int &local_port, const int &timeout_ms, const string &remote_addr, const int &remote_port);
        ~UDPClient()
    public:
        int async_send_to(char *data, const int &data_len);
        int async_receive(const int &data_len=0);

        int send_to(char *data, const int &data_len);
        int receive(const int &data_len=0);
    private:

};


