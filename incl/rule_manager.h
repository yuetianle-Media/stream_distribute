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
#include "pre_regex.h"
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
	int delay_time_ms;
	TASKCONTENT()
		:addr_cout(0), delay_time_ms(0)
	{
		memset(url, 0, sizeof(TASKCONTENT));
	}
};

typedef std::vector<TASKCONTENT> TASKCONTENTLIST;
typedef std::map<std::string, TASKCONTENT> TASKCONTENTMAP;/*<< url:task_content*/

typedef boost::lockfree::queue<TASKCONTENT, boost::lockfree::fixed_sized<true>> TASKTYPE;

typedef struct MRULECONTENT
{
	char url[256];
	IPPORT remote_addr;
	int delay_time_ms;
	MRULECONTENT()
		:delay_time_ms(0)
	{
		memset(url, 0, sizeof(url));
	}
}RULECONTENT;
typedef std::map<const string, const RULECONTENT> RULECONTENTTYPE;

using namespace pugi;
class RuleManager :public std::enable_shared_from_this<RuleManager>
{
	const char *URL_ATTR_NAME = "url";
	const char *IP_ATTR_NAME = "ip";
	const char *PORT_ATTR_NAME = "port";
	const char *URL_ATTR_DELAY_TIME = "delay_time";
public:
	RuleManager(const std::string &config_file);
	~RuleManager();

	bool get_task(TASKCONTENT &task_content);

	bool get_del_task(TASKCONTENT &task_content);

protected:
	/**
	 * @brief start_task start task to load file with a time interveral.
	 *
	 * @param in interveral unit:s
	 */
	void _start_task(const int interveral=5);

	/**
	 * @brief stop_task stop the task's load files.
	 */
	void _stop_task();
private:
	/**
	 * @brief _load_config_file load config files.
	 *
	 * @returns success:0 else error code(-1:read fail)
	 */
	int _load_config_file(const string &config_file);

	int _load_config_file(const string &config_file, RULECONTENTTYPE &cur_rules_content);

	int _get_add_task(TASKCONTENTLIST &add_task_list, RULECONTENTTYPE &cur_rules_content);

	int _push_add_task(const TASKCONTENTLIST &add_task_list);

	int _get_del_task(TASKCONTENTLIST &del_task_list, RULECONTENTTYPE &cur_rules_content);

	int _push_del_task(const TASKCONTENTLIST &del_task_list);

	void _do_task();
	void _do_task_ext();
	std::shared_ptr<thread> task_;
	bool b_exit_;
	pugi::xml_document doc_;
	std::string config_file_name_;
	TASKTYPE task_list_;
	TASKTYPE del_task_list_;
	int task_interval_;/*<< task interveal unit:s*/
	//std::map<const string, const string> stream_distribute_rules_;
	RULECONTENTTYPE stream_distribute_rules_;
	typedef std::map<const string, const RULECONTENT>::iterator stream_rule_iter;
};
typedef std::shared_ptr<RuleManager> RuleManagerPtr;
