// stream_distribute.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <boost/program_options.hpp>
#include "test_rule_manager.h"
#include "test_stream_receiver.h"
#include "test_stream_sender.h"
#include "test_udp_client.h"
#include "test_http_curl_client.h"
#include "test_stream_manager.h"
#include "test_tcp_client.h"
#include "test_m3u8_parser.h"
#include "version.h"
extern void test_rule_manager(const std::string &config_file);
extern void test_stream_receive(const std::string &url);
extern void test_stream_sender(const string &url, const string &multi_server, const int &port);
extern void test_udp_client(const string &addr, const int &port);
extern void test_http_curl_client();
extern void test_stream_manager(const string &config_file);
extern void test_stream_ts_callback(const std::string &ts_file_name);
extern void test_m3u8_content_parser();
inline int start_service(int argc, char* argv[])
{
	std::string program_title;
	program_title.append(argv[0]).append(" allow options");
	boost::program_options::options_description descrition(program_title);
	std::string cfg_file_name;
	descrition.add_options()
		("help,h", "produce help messages.")
		("version,v", "show the programs version")
#ifdef WIN32
		("file,f", boost::program_options::value<string>(&cfg_file_name)->default_value("rules.xml"), "load configuration files.")
#else
		("file,f", boost::program_options::value<string>(&cfg_file_name)->default_value("/etc/stream_distribute/rules.xml"), "load configuration files.")
#endif // WIN32
		("start", "start service")
		("stop", "stop service");
	boost::program_options::variables_map option_map;
	try
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, descrition), option_map);
	}
	catch (boost::program_options::error_with_no_option_name &e)
	{
		std::cout << "options error:" << e.what() << std::endl;
	}
	boost::program_options::notify(option_map);
	boost::filesystem::path default_cfg_file;
	if (option_map.count("help") || option_map.count("h"))
	{
		std::cout << descrition << std::endl;
	}
	else if (option_map.count("version") || option_map.count("v"))
	{
		std::cout << "program:" << argv[0] << " Version:" << GIT_VERSION << std::endl;
	}
	else if (option_map.count("start"))//启动服务
	{
		if (boost::filesystem::exists(boost::filesystem::path(cfg_file_name)))
		{
			std::cout << "start service cfg file:" << cfg_file_name << std::endl;
			StreamManager manager(cfg_file_name);
			manager.start();
			while (1)
			{
				std::this_thread::sleep_for(std::chrono::seconds(10));
			}
		}
		else
		{
			std::cout << "start service fail cfg file:" << cfg_file_name << " not exist!" << std::endl;
			return -1;
		}
	}
	else if (option_map.count("stop"))
	{
	}
	else
	{
		std::cout << "need to add options!!" << std::endl;
	}
	return 0;
}
int main(int argc, char **argv)
{
	start_service(argc, argv);
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
	//const string var_rate_uri = "http://125.88.92.166:30001/PLTV/88888956/224/3221227724/1.m3u8";
	//const string bug_uri = "http://125.88.92.166:30001/PLTV/88888956/224/3221227742/1.m3u8";
	//test_stream_receive(found_uri);
	//test_stream_receive(stream_uri);
	//test_stream_receive(var_rate_uri);
	//test_stream_receive(bug_uri);
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

	//test_stream_manager("rules.xml");


	//test_tcp_client();
    return 0;
}

