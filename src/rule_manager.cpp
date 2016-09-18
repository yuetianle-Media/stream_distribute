#include "stdafx.h"
#include "rule_manager.h"

RuleManager::RuleManager(const string &config_file)
    :config_file_name_(config_file)
	, task_(nullptr)
	, b_exit_(false)
	, task_list_(12800)
	, task_interval_(5)
{
}

RuleManager::~RuleManager()
{
	stop_task();
	this_thread::sleep_for(std::chrono::microseconds(1));
}

int RuleManager::_load_config_file(const string &config_file)
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
		pugi::xml_node root_node = doc_.first_child();
		for (pugi::xml_node_iterator it = root_node.begin(); it != root_node.end(); it++)
		{
			TASKCONTENT task_content;
			string url = it->attribute((char*)(RuleManager::URL_ATTR_NAME)).as_string();
			memcpy(task_content.url, url.data(), url.length());
			int ip_index = 0;
			int real_ip_index = 0;
			for (pugi::xml_node_iterator ip_node = it->begin(); ip_node != it->end(); ip_node++)
			{
				if (ip_index > MAX_DISTRIBUTE-1)//if ip more than max_distribute ignore the last ip.
					break;
				string ip = ip_node->attribute((char*)(RuleManager::IP_ATTR_NAME)).as_string();
				int port = ip_node->attribute((char*)(RuleManager::PORT_ATTR_NAME)).as_int();
				char stream_id_str[1024] = { 0 };
				sprintf(stream_id_str, "%s:%s:%d", url.data(), ip.data(), port);
				stream_rule_iter iter = stream_distribute_rules.find(stream_id_str);
				//if the rules has't add ignore it.
				if (iter == stream_distribute_rules.end())
				{
					memcpy(task_content.remote_addr_list[real_ip_index].ip, ip.data(), ip.length());
					task_content.remote_addr_list[real_ip_index].port = port;
					stream_distribute_rules.insert(make_pair(stream_id_str, ""));
					real_ip_index++;
				}
				ip_index++;
			}
			try
			{
				if (0 < real_ip_index)
				{
					task_content.addr_cout = real_ip_index;
					task_list_.push(task_content);//push the task to the queue.
				}
				else
				{
					//cout << "no tasks need add " << std::endl;
				}
			}
			catch(exception e)
			{
				cout << e.what() << std::endl;

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

void RuleManager::_do_task()
{
	while (1)
	{
		if (b_exit_)
			break;
		_load_config_file(config_file_name_);
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

void RuleManager::start_task(const int interveral/*=5*/)
{
	if (task_)
	{
		cout << "task is running thread id:" << task_->get_id() << std::endl;
		return;
	}
	//std::thread thread1((std::bind(&RuleManager::_do_task, this)));
	//task_ = std::make_shared<thread>(std::bind(&RuleManager::_do_task, shared_from_this()));
	task_.reset(new thread(std::bind(&RuleManager::_do_task, this)));
	if (task_)
	{
		task_->detach();
	}
}

void RuleManager::stop_task()
{
	b_exit_ = true;
	if (task_ && task_->joinable())
	{
		task_->join();
	}
	task_ == nullptr;
}
