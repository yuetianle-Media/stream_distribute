// stream_distribute.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "test_rule_manager.h"
#include "test_stream_receiver.h"
extern void test_rule_manager(const std::string &config_file);
extern void test_stream_receive(const std::string &url);
int main(int argc, char **argv)
{
	const string config_file = "rules.xml";
	//test_rule_manager(config_file);
	//while (true)
	//{
	//	this_thread::sleep_for(std::chrono::seconds(5));
	//}
	//const string stream_uri = "http://221.181.100.24:8088/envivo_v/SD/cctv/doc/711/01.m3u8";
	const string stream_uri = "http://192.168.203.131:9006/hls/test.m3u8";
	test_stream_receive(stream_uri);
	while (true)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}

    return 0;
}
