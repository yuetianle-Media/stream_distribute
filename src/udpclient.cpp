/* Copyright(C)
 * For free
 * All right reserved
 *
 */

UDPClient::UDPClient(const int &local_port, const int &timeout_ms, const string &remote_addr, const int &remote_port)
    :UDPSocketSession(local_port, timeout_ms)
{

}

