#include "stdafx.h"
#include "rule_manager.h"

RuleManager::RuleManager(const string &config_file)
    :config_file_name_(config_file)
	, task_(nullptr)
	, b_exit_(false)
	, task_list_(1024)
	, del_task_list_(1024)
	, task_interval_(5)
{
	//_start_task(task_interval_);
}

RuleManager::~RuleManager()
{
	_stop_task();
	this_thread::sleep_for(std::chrono::microseconds(1));
}

boost::signals2::connection RuleManager::subcribe_add_task(const ADD_TASK_SIGNAL::slot_type &slot)
{
	return add_task_signal_.connect(slot);
}

boost::signals2::connection RuleManager::subcribe_del_task(const ADD_TASK_SIGNAL::slot_type &slot)
{
	return del_task_signal_.connect(slot);
}

int RuleManager::_load_config_file(const string &config_file, RULECONTENTTYPE &cur_rules_content)
{
	if (config_file.empty())
	{
		return E_FILE_EMPTY;
	}
	boost::filesystem::path config_file_path(config_file);
	if (!boost::filesystem::exists(config_file_path) || !boost::filesystem::is_regular_file(config_file_path))
	{
		return E_FILE_ERROR;
	}
	pugi::xml_parse_result result = doc_.load_file(config_file_path.c_str());
	if (pugi::xml_parse_status::status_ok == result.status)
	{
		pugi::xml_node local_address_node = doc_.child("local_address");
		if (local_address_node)
		{
			v_lock(lk, local_ip_mtx);
			local_ip_ = local_address_node.attribute("ip").as_string();
		}
		pugi::xml_node rules_node = doc_.child("rules");
		for (pugi::xml_node_iterator it = rules_node.begin(); it != rules_node.end(); it++)
		{
			string url = it->attribute((char*)(RuleManager::URL_ATTR_NAME)).as_string();
			int task_type = it->attribute((char*)(RuleManager::TASK_TYPE_ATTR_NAME)).as_int();
			pugi::xml_attribute delay_time_attr =  it->attribute((char*)(RuleManager::URL_ATTR_DELAY_TIME));
			int delay_time = 0;
			if (delay_time_attr)
			{
				 delay_time = delay_time_attr.as_int();
			}
			int ip_index = 0;
			for (pugi::xml_node_iterator ip_node = it->begin(); ip_node != it->end(); ip_node++)
			{
				if (ip_index > MAX_DISTRIBUTE-1)//if ip more than max_distribute ignore the last ip.
					break;
				string ip = ip_node->attribute((char*)(RuleManager::IP_ATTR_NAME)).as_string();
				int port = ip_node->attribute((char*)(RuleManager::PORT_ATTR_NAME)).as_int();
				char stream_id_str[1024] = { 0 };
				sprintf(stream_id_str, "%s:%s:%d", url.data(), ip.data(), port);
				ip_index++;
				RULECONTENT rule_content;
				memcpy(rule_content.url, url.data(), url.length());
				memcpy(rule_content.remote_addr.ip, ip.data(), ip.length());
				rule_content.delay_time_ms = delay_time;
				rule_content.remote_addr.port = port;
				rule_content.task_type = (TaskType)task_type;
				cur_rules_content.insert(std::make_pair(stream_id_str, rule_content));
			}
		}
		doc_.reset();
		return E_OK;
	}
	else
	{
		return E_FILE_BAD_FORMAT;
	}
}

int RuleManager::_get_add_task(TASKCONTENTLIST &add_task_list, RULECONTENTTYPE &cur_rules_content)
{
	TASKCONTENTMAP task_map;
	//std::map<std::string, int> task_content_index;/*<< url ip index*/
	for (auto item : cur_rules_content)
	{
		stream_rule_iter iter = stream_distribute_rules_.find(item.first);
		if (iter == stream_distribute_rules_.end())//当前规则中没有此项规则
		{
			stream_distribute_rules_.insert(std::make_pair(item.first, item.second));
			TASKCONTENTMAP::iterator task_iter = task_map.find(item.second.url);
			if (task_iter == task_map.end())//没有此项内容
			{
				TASKCONTENT task;
				memcpy(task.url, item.second.url, sizeof(item.second.url));
				memcpy(task.remote_addr_list[0].ip, item.second.remote_addr.ip, sizeof(item.second.remote_addr.ip));
				task.remote_addr_list[0].port = item.second.remote_addr.port;
				task.delay_time_ms = item.second.delay_time_ms;
				task.task_type = item.second.task_type;
				//task_content_index.insert(std::make_pair(item.second.url, 1));
				task.addr_cout++;
				task_map.insert(std::make_pair(item.second.url, task));
			}
			else
			{
				//查找当前转发的IP地址的索引.
				int ip_index = task_iter->second.addr_cout;
				if (ip_index > MAX_DISTRIBUTE - 1)
				{
					cout << "more than the max:" << MAX_DISTRIBUTE << " count!!!" << std::endl;
					continue;
				}
				memcpy(task_iter->second.remote_addr_list[ip_index].ip\
						, item.second.remote_addr.ip\
						, sizeof(item.second.remote_addr.ip));
				task_iter->second.remote_addr_list[ip_index].port = item.second.remote_addr.port;
				task_iter->second.addr_cout++;
			}
		}
	}
	//将得到的任务写出去
	for (auto item : task_map)
	{
		add_task_list.push_back(item.second);
	}
	return E_OK;
}

int RuleManager::_push_add_task(const TASKCONTENTLIST &add_task_list)
{
	ostringstream out_stream;
	if (!add_task_list.empty())
		out_stream << "add list:" << std::endl;
	for (auto item : add_task_list)
	{
		//task_list_.push(item);
		add_task_signal_(item);
		out_stream << "url:" << item.url << "cout:" << item.addr_cout << std::endl;
		//vvlog_i("url:" << item.url << "cout:" << item.addr_cout);
		for (int index = 0; index < item.addr_cout; index++)
		{
			out_stream << "ip:" << item.remote_addr_list[index].ip << "port:" << item.remote_addr_list[index].port << std::endl;
			//vvlog_i("ip:" << item.remote_addr_list[index].ip << "port:" << item.remote_addr_list[index].port);
		}
	}
	string out_str = out_stream.str();
	if (!out_str.empty())
		vvlog_i(out_str);
	return E_OK;
}

int RuleManager::_get_del_task(TASKCONTENTLIST &del_task_list, RULECONTENTTYPE &cur_rules_content)
{
	TASKCONTENTMAP task_map;
	std::vector<std::string> key_list;
	for (auto item : stream_distribute_rules_)
	{
		stream_rule_iter iter = cur_rules_content.find(item.first);
		if (iter == cur_rules_content.end())//以前的规则在现在配置中没有
		{
			//stream_distribute_rules_.erase(item.first);
			key_list.push_back(item.first);
			TASKCONTENTMAP::iterator task_iter = task_map.find(item.second.url);
			if (task_iter == task_map.end())//没有此项内容
			{
				TASKCONTENT task;
				memcpy(task.url, item.second.url, sizeof(item.second.url));
				memcpy(task.remote_addr_list[0].ip, item.second.remote_addr.ip, sizeof(item.second.remote_addr.ip));
				task.remote_addr_list[0].port = item.second.remote_addr.port;
				task.addr_cout++;
				task_map.insert(std::make_pair(item.second.url, task));
			}
			else
			{
				//查找当前转发的IP地址的索引.
				int ip_index = task_iter->second.addr_cout;
				task_iter->second.addr_cout++;
				memcpy(task_iter->second.remote_addr_list[ip_index].ip\
						, item.second.remote_addr.ip\
						, sizeof(item.second.remote_addr.ip));
				task_iter->second.remote_addr_list[ip_index].port = item.second.remote_addr.port;
			}
		}
	}
	//擦除数据
	for (auto item : key_list)
	{
		stream_distribute_rules_.erase(item);
	}
	//将得到的任务写出去
	for (auto item : task_map)
	{
		del_task_list.push_back(item.second);
	}
	return E_OK;
}

int RuleManager::_push_del_task(const TASKCONTENTLIST &del_task_list)
{
	if (!del_task_list.empty())
		cout << "del list:" << std::endl;
	for (auto item : del_task_list)
	{
		cout << "url:" << item.url << " cout:" << item.addr_cout << std::endl;
		del_task_signal_(item);
		//del_task_list_.push(item);
		for (int index = 0; index < item.addr_cout; index++)
		{
			cout << "ip:" << item.remote_addr_list[index].ip << "port:" << item.remote_addr_list[index].port << std::endl;
		}
	}
	return E_OK;
}

void RuleManager::_do_task_ext()
{
	vvlog_i("doing back process config file:" << config_file_name_ << "pid:" << std::this_thread::get_id());
	while (1)
	{
		if (b_exit_)
			break;
		RULECONTENTTYPE rule_content;
		_load_config_file(config_file_name_, rule_content);
		TASKCONTENTLIST add_list;
		_get_add_task(add_list, rule_content);
		_push_add_task(add_list);
		TASKCONTENTLIST del_list;
		_get_del_task(del_list, rule_content);
		_push_del_task(del_list);
		this_thread::sleep_for(std::chrono::seconds(task_interval_));
	}

}

bool RuleManager::get_task(TASKCONTENT &task_content) 
{
	if (task_list_.pop(task_content))
	{
		return true;
	}
	else
	{
		//std::cout << "no tasks !!" << std::endl;
		return false;
	}
}

bool RuleManager::get_del_task(TASKCONTENT &task_content)
{
	if (del_task_list_.pop(task_content))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool RuleManager::get_local_ip(std::string &local_ip)
{
	if (!local_ip_.empty())
	{
		local_ip = local_ip_;
		return true;
	}
	else
	{
		return false;
	}
}

void RuleManager::start_task(const int interveral/*=5*/)
{
	if (task_)
	{
		cout << "task is running thread id:" << task_->get_id() << std::endl;
		return;
	}
	task_.reset(new thread(std::bind(&RuleManager::_do_task_ext, this)));
	if (task_)
	{
		vvlog_i("start process config file:" << config_file_name_ << " pid:" << std::this_thread::get_id());
	}
}
void RuleManager::_start_task(const int interveral/*=5*/)
{
	if (task_)
	{
		cout << "task is running thread id:" << task_->get_id() << std::endl;
		return;
	}
	task_.reset(new thread(std::bind(&RuleManager::_do_task_ext, this)));
	if (task_)
	{
		vvlog_i("start process config file:" << config_file_name_ << " pid:" << std::this_thread::get_id());
		//task_->detach();
	}
}

void RuleManager::_stop_task()
{
	b_exit_ = true;
	if (task_ && task_->joinable())
	{
		task_->join();
	}
	task_ == nullptr;
}
