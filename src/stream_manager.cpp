#include "stdafx.h"
#include "stream_manager.h"


StreamManager::StreamManager(const string &config_file)
	:rule_config_file_(config_file), add_task_(nullptr)\
	, remove_task_(nullptr), is_exit_(false)
{
	rules_manager_.reset(new RuleManager(config_file));
}

StreamManager::~StreamManager()
{
}

bool StreamManager::start()
{
	if (!add_task_)
	{
		add_task_.reset(new std::thread(std::bind(&StreamManager::_do_add_tasks_callback, this)));
	}
	if (!remove_task_)
	{
		remove_task_.reset(new std::thread(std::bind(&StreamManager::_do_del_tasks_callback, this)));
	}
	if (add_task_ && remove_task_)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool StreamManager::stop()
{
	is_exit_ = true;
	if (add_task_ && add_task_->joinable())
	{
		add_task_->join();
	}
	add_task_ = nullptr;
	if (remove_task_ && remove_task_->joinable())
	{
		remove_task_->join();
	}
	remove_task_ = nullptr;
	return true;
}

void StreamManager::_do_add_tasks_callback()
{
	if (rules_manager_)
	{
		while (1)
		{
			TASKCONTENT task_content;
			while (rules_manager_->get_task(task_content))
			{
				_do_add_task(task_content);
			}
			this_thread::sleep_for(chrono::milliseconds(1));
			if (is_exit_)
			{
				break;
			}
		}
	}
}

void StreamManager::_do_del_tasks_callback()
{
	if (rules_manager_)
	{
		while (1)
		{
			TASKCONTENT task_content;
			while (rules_manager_->get_del_task(task_content))
			{
				_do_remove_task(task_content);
			}
			this_thread::sleep_for(chrono::milliseconds(1));
			if (is_exit_)
			{
				break;
			}
		}
	}

}

bool StreamManager::_do_add_task(const TASKCONTENT &task_content)
{
	StreamReceiverPtr receiver_ptr = nullptr;
	StreamSenderPtr sender_ptr = nullptr;
	StreamContent stream_content;
	if (0 < strlen(task_content.url))
	{
		 receiver_ptr = make_shared<StreamReceiver>(task_content.url);
	}
	if (0 < task_content.addr_cout)
	{
		sender_ptr = make_shared<StreamSender>();
	}
	for (int index = 0; index < task_content.addr_cout; index++)
	{
		sender_ptr->add_sender_address(task_content.remote_addr_list[index].ip\
			, task_content.remote_addr_list[index].port);
		char ip_port_id[100] = { 0 };
		sprintf(ip_port_id, "%s:%d", task_content.remote_addr_list[index].ip\
			, task_content.remote_addr_list[index].port);
		stream_content.multi_addr.insert(std::make_pair(ip_port_id, task_content.remote_addr_list[index]));
	}
	if (receiver_ptr)
		receiver_ptr->start();
	if (sender_ptr)
		sender_ptr->start();
	if (receiver_ptr && sender_ptr)
	{
		stream_content.sender = sender_ptr;
		stream_content.receiver = receiver_ptr;
		receiver_ptr->subcribe_ts_callback(boost::bind(&StreamSender::stream_receive_callback, sender_ptr, _1, _2,_3));
		v_lock(lk, stream_map_mutex_);
		stream_map_.insert(std::make_pair(task_content.url, stream_content));
	}
	return true;
}

bool StreamManager::_do_remove_task(const TASKCONTENT &task_content)
{
	v_lock(lk, stream_map_mutex_);
	StreamMap::iterator iter = stream_map_.find(task_content.url);
	if (iter != stream_map_.end())
	{

		for (int task_index = 0; task_index < task_content.addr_cout; task_index++)
		{
			iter->second.sender->del_sender_address(\
				task_content.remote_addr_list[task_index].ip\
				, task_content.remote_addr_list[task_index].port);
		}
		if (task_content.addr_cout == iter->second.multi_addr.size())//无转发任务
		{
			iter->second.receiver->stop();
			iter->second.sender->stop();
			stream_map_.erase(task_content.url);//从流表中删除
		}
	}
	return true;
}
