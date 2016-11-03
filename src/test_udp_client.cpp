#include "stdafx.h"
#include "test_udp_client.h"
#include "tspacket.h"
#include "stream_sender_buffer.h"
#include "stream_sender.h"
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

boost::lockfree::queue<TSSENDCONTENT, boost::lockfree::fixed_sized<true>> ts_send_content_queue_(30000);
void test_queue()
{
	TS_SEND_CONTENT me1;
	TS_SEND_CONTENT me2;
	bool is_ok = false;
	is_ok = ts_send_content_queue_.push(me1);
	assert(is_ok == true);
	is_ok = ts_send_content_queue_.push(me2);
	assert(is_ok == true);
	TS_SEND_CONTENT out;
	is_ok = ts_send_content_queue_.pop(out);
	assert(is_ok == true);
	TS_SEND_CONTENT me3;
	is_ok = ts_send_content_queue_.push(me2);
	assert(is_ok == true);
		
}
void read_ts_func()
{
	fstream ss;
	ss.open("20161017T142630.ts", ios::in | ios::binary);
	FILE* fd = fopen("20161017T142630.ts", "rb");
	if (!fd)
	{
		assert(false);
	}
	char ts_send_buffer[188];
	long int pcr_duration_packet_num_ =0;
	long int ts_packet_num_ = 0;
	long int pcr_first_packet_num = 0;
	long int pcr_second_packet_num = 0;
	bool is_first_pcr_ = false;
	PCR pcr_front_ = 0;
	PCR pcr_end_ = 0;
	PCR max_pcr_ = 0;
	StreamSenderBuffer sender_buffer_;
	ss.seekg(0, ss.end);
	long int length = ss.tellg();
	ss.seekg(0, ss.beg);
	long int all_queue_size = 0;
	while (0 < length)
	{
		//if (ss.read(ts_send_buffer, 188))
		if (0 < fread(ts_send_buffer,1,188, fd))
		{
			length -= 188;
			CTsPacket ts_packet_;
			if (ts_packet_.SetPacket((BYTE*)ts_send_buffer))
			{

				PCR packet_pcr = ts_packet_.Get_PCR();
				WORD pid = ts_packet_.Get_PID();
				//stream_buffer_.pushToBuffer(ts_data.content, ts_data.real_size);
				int result = sender_buffer_.push_to_buffer(ts_send_buffer, 188);
				if (E_OK != result)
				{
					vvlog_e("push to sender buff fail, error:" << result);
				}
				ts_packet_num_++;
				//pcr_duration_packet_num_++;
				if (INVALID_PCR != packet_pcr)
				{
					if (!is_first_pcr_)//
					{
						is_first_pcr_ = true;
						pcr_front_ = pcr_end_ = packet_pcr;
						pcr_first_packet_num = ts_packet_num_;
					}
					else
					{
						pcr_front_ = pcr_end_;
						pcr_end_ = packet_pcr;
						if (pcr_end_ < pcr_front_)
						{
							max_pcr_ = pcr_front_;
							pcr_front_ = 0;
						}
						pcr_second_packet_num = ts_packet_num_;
						pcr_duration_packet_num_ = pcr_second_packet_num - pcr_first_packet_num;
						long int system_clock_reference = 27;
						double tranlate_rate_ = (double)(8*pcr_duration_packet_num_*188*27) / (pcr_end_ - pcr_front_);
						double tranlate_interval_time_ = (double)(188*7 *8)/(long)(1000*tranlate_rate_);//发送188*7需要的时间
						long int tmp_data_len = sender_buffer_.get_data_length();
						char *tmp_data = sender_buffer_.get_data();
						pcr_first_packet_num = pcr_second_packet_num = ts_packet_num_ = 0;
						while (188*7 < tmp_data_len)
						{
							TS_SEND_CONTENT send_content;
							memcpy(send_content.content, tmp_data, 188 * 7);
							send_content.real_size = 188 * 7;
							send_content.need_time = tranlate_interval_time_;
							all_queue_size++;
							
							if (!ts_send_content_queue_.push(send_content))
							{
								std::cout << "push queue fail." << std::endl;
							}
							else
							{
								this_thread::sleep_for(std::chrono::microseconds(1));
							}
							tmp_data += 188*7;
							tmp_data_len -= (188*7);
						}
						if (0 < tmp_data_len && tmp_data)
						{
							TS_SEND_CONTENT send_content_second;
							memcpy(send_content_second.content, tmp_data, tmp_data_len);
							send_content_second.real_size = tmp_data_len;
							int send_time = (tranlate_interval_time_*tmp_data_len / (188*7))*1000;
							send_content_second.need_time = send_time*1000;
							all_queue_size++;
							if (!ts_send_content_queue_.push(send_content_second))
							{
								std::cout << "push queue fail 2." << std::endl;
							}
							else
							{
								this_thread::sleep_for(std::chrono::microseconds(1));
							}
							
							//client->write_ext(tmp_data, 188 * 7, send_time, "244.1.1.1", 65002);
						}
						sender_buffer_.reset_buffer();
					}
				}
			}
			else
			{
				std::cout << "invaild packet" << std::endl;
			}
			memset(ts_send_buffer, 0, 188);
		}
	}
	std::cout << "queue size:" << all_queue_size << std::endl;
	while (true)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}
	if (fd)
	{
		fclose(fd);
	}
}
void test_udp_client(const string &addr, const int &port)
{
	UDPClientPtr client = make_shared<UDPClient>(0,5000,"224.1.1.1", 65002);

	std::shared_ptr<std::thread> read_thread_ptr = nullptr;
	read_thread_ptr.reset(new std::thread(std::bind(read_ts_func)));
	read_thread_ptr->detach();
	//client->connect();
	while (1)
	{
		TS_SEND_CONTENT ts_content;
		while (ts_send_content_queue_.pop(ts_content))
		{
			//client->write(ts_content.content, ts_content.real_size, ts_content.need_time);
			client->write_ext(ts_content.content\
				, ts_content.real_size\
				, ts_content.need_time\
				, "224.1.1.1"\
				, 65002);
		}
		this_thread::sleep_for(std::chrono::microseconds(1));
	}
}
