#include "stdafx.h"
#include "test_udp_client.h"

void test_udp_sync_client()
{
	std::shared_ptr<boost::asio::io_service> io_service = make_shared<boost::asio::io_service>();

	udp::socket s(*io_service, udp::endpoint(udp::v4(), 0));

	udp::resolver resolver_one(*io_service);
	udp::endpoint endpoint(boost::asio::ip::address::from_string("224.0.2.190"), 5000);

	std::cout << "Enter message: ";
	char request[1024] = {0};
	std::cin.getline(request, 1024);
	size_t request_length = std::strlen(request);
	s.send_to(boost::asio::buffer(request, request_length), endpoint);

	char reply[1024];
	udp::endpoint sender_endpoint;
	size_t reply_length = s.receive_from(
		boost::asio::buffer(reply, 1024), sender_endpoint);
	std::cout << "Reply is: ";
	std::cout.write(reply, reply_length);
	std::cout << "\n";

}

void open_ts_file(const string &file_name)
{
	
}
void test_udp_client(const string &addr, const int &port)
{
	//UDPClientPtr client = make_shared<UDPClient>(0,5, "224.0.2.190", 6000);
	UDPClientPtr client = make_shared<UDPClient>(0,5,"224.1.1.1", 65000);
	//UDPClientPtr client = make_shared<UDPClient>(0,5,"10.160.35.125", 60000);

	fstream ss;
	ss.open("ts.ts", ios::in | ios::binary);
	char ts_send_buffer[188];
	client->connect();
	while (1)
	{
		if (ss.read(ts_send_buffer, 188))
		{
			client->write(ts_send_buffer, 188);
			this_thread::sleep_for(chrono::milliseconds(10));
			memset(ts_send_buffer, 0, 188);
		}
	}

}
