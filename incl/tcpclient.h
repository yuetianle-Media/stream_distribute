#include "socket_session.h"


using namespace boost;
typedef boost::signals2::signal<void(char* data, const &data_len)> DataReceiveSignal;
class TCPClient: public SocketSession
{
public:
	TCPClient(const string &remote_server, const int &port, const int &time_out/*s*/);
    ~TCPClient();
    virtual int send (const char *content, const int &length)override;
    virtual int receive()override;
	virtual int async_send(const char *content, const int &length)override;
	virtual int async_receive()override;

	virtual int wait_response();
private:
	int _send(const char *content, const int &length, boost::asio::yield_context yield);
	int _receive(const int &size);

};
