/* Copyright(C)
 * For free
 * All right reserved
 *
 */
#pragma once
#include "pugixml.hpp"
#include <boost/lockfree/queue.hpp>
#include <boost/filesystem.hpp>
#include "errcode.h"
#include "pre_std_basic.h"

#include "pre_boost_basic.h"
using namespace std;


struct IPPORT
{
	char ip[16];
	int port;
	IPPORT()
	{
		memset(ip, 0, sizeof(IPPORT));
	}
};

#define MAX_DISTRIBUTE 50
struct TASKCONTENT
{
	char url[256];
	IPPORT remote_addr_list[MAX_DISTRIBUTE];
	int addr_cout;
	TASKCONTENT()
		:addr_cout(0)
	{
		memset(url, 0, sizeof(TASKCONTENT));
	}
};


typedef boost::lockfree::queue<TASKCONTENT, boost::lockfree::fixed_sized<true>> TASKTYPE;
using namespace pugi;
class RuleManager :public enable_shared_from_this<RuleManager>
{
	const char *URL_ATTR_NAME = "url";
	const char *IP_ATTR_NAME = "ip";
	const char *PORT_ATTR_NAME = "port";
public:
	RuleManager(const std::string &config_file);
	~RuleManager();

	/**
	 * @brief start_task start task to load file with a time interveral.
	 *
	 * @param in interveral unit:s
	 */
	void start_task(const int interveral=5);

	/**
	 * @brief stop_task stop the task's load files.
	 */
	void stop_task();
	bool get_task(TASKCONTENT &task_content);
private:
	/**
	 * @brief _load_config_file load config files.
	 *
	 * @returns success:0 else error code(-1:read fail)
	 */
	int _load_config_file(const string &config_file);


	void _do_task();

	std::shared_ptr<thread> task_;
	bool b_exit_;
	pugi::xml_document doc_;
	std::string config_file_name_;
	TASKTYPE task_list_;
	int task_interval_;/*<< task interveal unit:s*/
	std::map<const string, const string> stream_distribute_rules;
	typedef std::map<const string, const string>::iterator stream_rule_iter;
};
