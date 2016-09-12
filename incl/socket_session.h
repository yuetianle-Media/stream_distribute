#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;

class SocketSession: public boost::enable_shared_from_this<SocketSession>
{
    public:
        SocketSession(const string &remote_server, const int &port, const int &time_out/*s*/);
        ~SocketSession();

        virtual int send(const char *content, const int &length)=0;
        virtual int receive()=0;

        virtual int async_send(const char *content, const int &length)=0;
        virtual int async_receive()=0;

        //void handle_write(const boost::system::error_code &ec, size_t byte_transferred);
        //void handle_read(const boost::system::error_code &ec);
    private:
        void connect_handler(const boost::system::error_code &ec);

        boost::asio::io_service io_svt_;
        boost::shared_ptr<boost::asio::io_service::work> work_ptr_;
        boost::shared_ptr<boost::asio::strand> strand_ptr_;

        boost::asio::ip::tcp::endpoint remote_addr;
        boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr_;
        //boost::asio::deadline_timer timer_;

        int _connect();
        int _async_connect();


};

typedef boost::shared_ptr<SocketSession> session_ptr;
