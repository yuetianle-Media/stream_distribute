 /**
 * @file udp_socket_session.h
 * @brief udp socket class
 * @author lee, shida23577@hotmail.com
 * @version 1.0.0
 * @date 2016-09-14
 */

using namespace boost::asio::ip::udp

class UDPSocketSession :public enabled_shared_from_this<UDPSocketSession>
{
    public:
        UDPSocketSession(const int &local_port, const int &timeout_ms);
        virtual ~UDPSocketSession();
    protected:
        udp::socket socket_;
        udp::endpoint remote_ep_;
        int local_port_;
        int time_out_ms_;
        shared_ptr<boost::asio::io_service> io_svt_;
        shared_ptr<boost::io_service::work> work_;
        boost::asio::io_service::strand strand_;
}

typedef shared_ptr<UDPSocketSession> UDPSocketSessionPtr;
