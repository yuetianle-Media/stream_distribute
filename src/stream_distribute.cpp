// stream_distribute.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_rule_manager.h"
#include "test_stream_receiver.h"
#include "test_stream_sender.h"
#include "test_udp_client.h"
extern void test_rule_manager(const std::string &config_file);
extern void test_stream_receive(const std::string &url);
extern void test_stream_sender(const string &url, const string &multi_server, const int &port);
extern void test_udp_client(const string &addr, const int &port);

extern void test_stream_ts_callback(const std::string &ts_file_name);
int main(int argc, char **argv)
{
	//test_stream_ts_callback("send_ts_39.ts");
	//return 0;


	//const string config_file = "rules.xml";
	//test_rule_manager(config_file);
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::seconds(5));
	//}



	//const string stream_uri = "http://221.181.100.24:8088/envivo_v/SD/cctv/doc/711/01.m3u8";
	//const string stream_uri = "http://192.168.203.211:9006/hls/test.m3u8";
	//const string world_stream_uri = "http://221.181.100.24:8088/ws_v/sjdl/sjdl711/711/01.m3u8";
	//test_stream_receive(stream_uri);
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::seconds(5));
	//}


	const string stream_uri = "http://192.168.203.211:9006/hls/test.m3u8";
	const string cctv_music = "http://58.135.196.138:8090/live/ADBD55B50F2D47bb970DCCBAF458E6C8/index.m3u8";
	const string cctv_5 = "http://10.160.34.115:91/119/119.m3u8";
	//test_stream_sender(stream_uri, "224.1.1.1", 65000);
	test_stream_sender(cctv_5, "224.1.1.1", 65000);
	while (true)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}


	//test_udp_client("224.0.2.100", 9000);
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::seconds(5));
	//}
    return 0;
}

