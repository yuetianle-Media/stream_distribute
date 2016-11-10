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

//BOOL get_pcr(const PBYTE p_ts_packet, INT64* p_pcr, INT* p_pcr_pid)
//{
//	if ((p_ts_packet[0] == 0x47) &&
//		(p_ts_packet[3] & 0x20) &&
//		(p_ts_packet[5] & 0x10) &&
//		(p_ts_packet[4] >= 7))
//	{
//		*p_pcr_pid = ((INT)p_ts_packet[1] & 0x1F) << 8 | p_ts_packet[2];
//
//		*p_pcr = ((INT64)p_ts_packet[6] << 25) |
//			((INT64)p_ts_packet[7] << 17) |
//			((INT64)p_ts_packet[8] << 9) |
//			((INT64)p_ts_packet[9] << 1) |
//			((INT64)p_ts_packet[10] >> 7);
//		return TRUE;
//	}
//
//	return FALSE;
//}

#define SLEEP_COUNT_TEST 250
//boost::lockfree::queue<TSSENDCONTENT, boost::lockfree::fixed_sized<false>> ts_send_content_queue_(30000);
boost::lockfree::spsc_queue<TSSENDCONTENT, boost::lockfree::fixed_sized<false>> ts_send_content_queue_(30000);
//boost::lockfree::queue<TSPACKETCONTENT, boost::lockfree::fixed_sized<false>> ts_packet_queue(30000);

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
	ss.open("20161107T100344.ts", ios::in | ios::binary);
	FILE* fd = fopen("20161107T100344.ts", "rb");
	if (!fd)
	{
		assert(false);
	}
	std::cout << "come herer" << std::endl;
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
				//WORD pid = ts_packet.Get_PID();
				int result = sender_buffer_.push_to_buffer(ts_send_buffer, 188);
				if (E_OK != result)
				{
					vvlog_e("push ts data to sender buff fail, error:" << result);
				}
				ts_packet_num_++;
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
						double tranlate_rate = (double)(8 * pcr_duration_packet_num_*TS_PACKET_LENGTH_STANDARD*system_clock_reference) / (pcr_end_ - pcr_front_);
						double tranlate_interval_time = (double)(TS_PACKET_LENGTH_STANDARD * 7 * 8*1000) / (long)(tranlate_rate);//发送188*7需要的时间 micro
						//vvlog_i("url:" << play_stream_ << "tsNum:" << pcr_duration_packet_num << "first_pcr:" << pcr_front << "pcr_end:" << pcr_end\
							<< "rate:" << tranlate_rate_byte_micro << "time:" << tranlate_interval_time);
						pcr_first_packet_num = pcr_second_packet_num = ts_packet_num_ = 0;
						//_push_ts_data_to_send_queue(sender_buffer.get_data(), sender_buffer.get_data_length(), tranlate_interval_time);
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
	client->resize_send_buffer_size(1024 * 1024 * 1.25);
	//client->connect();
	client->set_reuse(true);
	client->set_debug(true);
	std::shared_ptr<std::thread> read_thread_ptr = nullptr;
	read_thread_ptr.reset(new std::thread(std::bind(read_ts_func)));
	read_thread_ptr->detach();
	//client->connect();
	//client->resize_send_buffer_size(0);
	std::thread task_thread([&]() {
	TS_SEND_CONTENT ts_content;
	TSPACKETCONTENT ts_data;

	char send_data[188 * 7] = { 0 };
	int send_index = 0;
	int64_t start_time = 0;
	PCR first_pcr = 0;
	int64_t cur_time = 0;
	PCR cur_pcr = 0;
	int64_t success_time = 0;
	bool is_first = false;
	long int sleep_time = 0;
	DWORD cur_time_d = 0;
	DWORD start_time_d = 0;
	int sleep_cout = SLEEP_COUNT_TEST;
	int64_t all_run_time = 0;
	int64_t all_need_time = 0;
	bool is_first_send = false;
	while (1)
	{

		while(ts_send_content_queue_.pop(ts_content))
		{
			start_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			int result = client->write_ext(ts_content.content\
				, ts_content.real_size\
				, ts_content.need_time\
				, "224.1.1.1"\
				, 65002);
			if (result == E_OK)
			{
				success_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				int64_t run_time_count = success_time - start_time;
				all_run_time += run_time_count;
				all_need_time += ts_content.need_time;
				sleep_cout -= 1;
				if (0 == sleep_cout)
				{
					if (all_need_time > all_run_time)
					{
						int64_t sleep_time = int64_t((all_need_time-all_run_time));
						this_thread::sleep_for(std::chrono::nanoseconds((long)(sleep_time)));
					}
					sleep_cout = SLEEP_COUNT_TEST;
					all_run_time = 0;
					all_need_time = 0;
				}
			}
			else
			{
				std::cout << "send faile" << std::endl;
			}
		}
		//this_thread::sleep_for(std::chrono::nanoseconds(1));
	}
	});
	while (true)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}
}
