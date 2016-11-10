// stream_distribute.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_rule_manager.h"
#include "test_stream_receiver.h"
#include "test_stream_sender.h"
#include "test_udp_client.h"
#include "test_http_curl_client.h"
#include "test_stream_manager.h"
#include "test_tcp_client.h"
#include "test_m3u8_parser.h"
extern void test_rule_manager(const std::string &config_file);
extern void test_stream_receive(const std::string &url);
extern void test_stream_sender(const string &url, const string &multi_server, const int &port);
extern void test_udp_client(const string &addr, const int &port);
extern void test_http_curl_client();
extern void test_stream_manager(const string &config_file);
extern void test_stream_ts_callback(const std::string &ts_file_name);
extern void test_m3u8_content_parser();
int main(int argc, char **argv)
{
	//test_m3u8_content_parser();
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::seconds(5));
	//}

	//const string config_file = "rules.xml";
	//test_rule_manager(config_file);
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::seconds(5));
	//}


	//test_http_curl_client();

	//const string stream_uri = "http://10.160.34.115:91/119/119.m3u8";
	////const string cctv_6_h265 = "http://10.160.34.115:91/120/120.m3u8";
	////const string found_uri = "http://58.20.59.58:8020/live/fazhilv.stream_aac/playlist.m3u8";
	////test_stream_receive(found_uri);
	//test_stream_receive(stream_uri);
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::seconds(5));
	//}

	
	//const string stream_uri = "http://192.168.203.211:9006/hls/test.m3u8";
	//const string cctv_5 = "http://10.160.34.115:91/119/119.m3u8";
	//test_stream_sender(cctv_5, "224.1.1.1", 65002);
	////test_stream_sender(stream_uri, "224.1.1.1", 65002);
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::seconds(5));
	//}


#ifdef WIN32
	//test_udp_client("224.0.2.100", 9000);
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::seconds(5));
	//}
#endif
	
	//test_async_task();

	test_stream_manager("rules.xml");


	//test_tcp_client();
    return 0;
}

