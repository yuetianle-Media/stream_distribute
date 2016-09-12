#include "socket_session.h"


class TCPClient: public SocketSession
{
    TCPClient(const string &remote_server, const int &time_out);
    ~TCPClient();
}
