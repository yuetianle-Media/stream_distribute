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
	if (rules_manager_)
	{
	rules_manager_->subcribe_add_task(boost::bind(&StreamManager::_do_add_task_signal, this, _1));
	rules_manager_->subcribe_del_task(boost::bind(&StreamManager::_do_del_task_signal, this, _1));
		rules_manager_->start_task();
	return true;
	}
	else
	{
		return false;
	}
	//if (!add_task_)
	//{
	//	add_task_.reset(new std::thread(std::bind(&StreamManager::_do_add_tasks_callback, this)));
	//}
	//if (!remove_task_)
	//{
	//	remove_task_.reset(new std::thread(std::bind(&StreamManager::_do_del_tasks_callback, this)));
	//}
	//if (add_task_ && remove_task_)
	//{
	//	add_task_->detach();
	//	remove_task_->detach();
	//	return true;
	//}
	//else
	//{
	//	return false;
	//}
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

void StreamManager::_do_add_task_signal(const TASKCONTENT&task_content)
{
	std::string multi_addr;
	std::ostringstream ss_out;
	for (int index=0; index < task_content.addr_cout; index++)
	{
		ss_out << task_content.remote_addr_list[index].ip\
			<< ":" << task_content.remote_addr_list[index].port << ";";
	}
	multi_addr = ss_out.str();
	std::string local_ip;
	bool add_success = false;
	if (rules_manager_->get_local_ip(local_ip) && !local_ip.empty())
	{
		if (task_content.task_type == FILE_TASK)
		{
			add_success = _do_add_ts_task(task_content, local_ip);
		}
		else
		{
		add_success = _do_add_task(task_content, local_ip);
		}
	}
	else
	{
		if (task_content.task_type == FILE_TASK)
		{
			add_success = _do_add_ts_task(task_content);
		}
		else
		{
		add_success = _do_add_task(task_content);
		}
	}
	if (add_success)
	{
		vvlog_i("add stream task success url:" << task_content.url\
			<< "multiaddress cout:" << task_content.addr_cout\
			<< "address:" << multi_addr << "thread id:" << std::this_thread::get_id());
	}
	else
	{
		vvlog_e("add stream task fail url:" << task_content.url\
			<< "multiaddress cout:" << task_content.addr_cout\
			<< "address:" << multi_addr<< "thread id:" << std::this_thread::get_id());
	}
}

void StreamManager::_do_del_task_signal(const TASKCONTENT&task_content)
{
	std::string multi_addr;
	std::ostringstream ss_out;
	for (auto item : task_content.remote_addr_list)
	{
		ss_out << item.ip << ":" << item.port;
	}
	multi_addr = ss_out.str();
	if (_do_remove_task(task_content))
	{
		vvlog_i("del stream task success url:" << task_content.url\
			<< "multiaddress cout:" << task_content.addr_cout\
			<< "address:" << multi_addr);
	}
	else
	{
		vvlog_e("del stream task fail url:" << task_content.url\
			<< "multiaddress cout:" << task_content.addr_cout\
			<< "address:" << multi_addr);
	}

}

void StreamManager::_do_add_tasks_callback()
{
	if (rules_manager_)
	{
		TASKTYPE *add_task_queue = nullptr;
		rules_manager_->get_add_task_queue(add_task_queue);
		std::cout << "add tasks pid:" << std::this_thread::get_id() << std::endl;
		while (1)
		{
			TASKCONTENT task_content;
			while (rules_manager_->get_task(task_content))
			{
				std::string multi_addr;
				std::ostringstream ss_out;
				for (int index=0; index < task_content.addr_cout; index++)
				{
					ss_out << task_content.remote_addr_list[index].ip\
						<< ":" << task_content.remote_addr_list[index].port << ";";
				}
				multi_addr = ss_out.str();
				std::string local_ip;
				bool add_success = false;
				if (rules_manager_->get_local_ip(local_ip) && !local_ip.empty())
				{
					add_success = _do_add_task(task_content, local_ip);
				}
				else
				{
					add_success = _do_add_task(task_content);
				}
				if (add_success)
				{
					vvlog_i("add stream task success url:" << task_content.url\
						<< "multiaddress cout:" << task_content.addr_cout\
						<< "address:" << multi_addr);
				}
				else
				{
					vvlog_e("add stream task fail url:" << task_content.url\
						<< "multiaddress cout:" << task_content.addr_cout\
						<< "address:" << multi_addr);
				}
				//Sleep(1000);
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			std::this_thread::sleep_for(std::chrono::seconds(5));
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
		std::cout << "del tasks callback pid:" << std::this_thread::get_id() << std::endl;
		while (1)
		{
			TASKCONTENT task_content;
			while (rules_manager_->get_del_task(task_content))
			{
				std::string multi_addr;
				std::ostringstream ss_out;
				for (auto item : task_content.remote_addr_list)
				{
					ss_out << item.ip << ":" << item.port;
				}
				multi_addr = ss_out.str();
				if (_do_remove_task(task_content))
				{
					vvlog_i("del stream task success url:" << task_content.url\
						<< "multiaddress cout:" << task_content.addr_cout\
						<< "address:" << multi_addr);
				}
				else
				{
					vvlog_e("del stream task fail url:" << task_content.url\
						<< "multiaddress cout:" << task_content.addr_cout\
						<< "address:" << multi_addr);
				}
			}
			std::this_thread::sleep_for(std::chrono::seconds(5));
			if (is_exit_)
			{
				break;
			}
		}
	}

}

bool StreamManager::_do_add_task(const TASKCONTENT &task_content, const std::string &local_addr)
{
	//return true;
	StreamReceiverPtr receiver_ptr = nullptr;
	StreamSenderPtr sender_ptr = nullptr;
	StreamContent stream_content;
	v_lock(lk, stream_map_mutex_);
	if (0 >= strlen(task_content.url))
	{
		return false;
	}
	StreamMap::iterator iter = stream_map_.find(task_content.url);
	if (iter == stream_map_.end())//没有此视频流
	{
		receiver_ptr = std::make_shared<StreamReceiver>(task_content.url);
		if (0 < task_content.addr_cout)
		{
			//sender_ptr = std::make_shared<StreamSender>();
			
			if (!local_addr.empty())
			{
				sender_ptr = std::make_shared<StreamSender>(receiver_ptr, local_addr);
				sender_ptr->set_local_ip(local_addr);
			}
			else
			{
				sender_ptr = std::make_shared<StreamSender>(receiver_ptr);
			}
			sender_ptr->set_delay_time(task_content.delay_time_ms);
			sender_ptr->set_receive_url(task_content.url);
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
			//std::async(std::launch::async, [&]() {
			//});
			if (receiver_ptr && sender_ptr)
			{
				stream_content.sender = sender_ptr;
				stream_content.receiver = receiver_ptr;
				stream_map_.insert(std::make_pair(task_content.url, stream_content));
			}
		}
		else//没有转发组播IP
		{
			return false;
		}
	}
	else
	{
		sender_ptr = iter->second.sender;
		for (int index = 0; index < task_content.addr_cout; index++)
		{
			sender_ptr->add_sender_address(task_content.remote_addr_list[index].ip\
				, task_content.remote_addr_list[index].port);
			char ip_port_id[100] = { 0 };
			sprintf(ip_port_id, "%s:%d", task_content.remote_addr_list[index].ip\
				, task_content.remote_addr_list[index].port);
			iter->second.multi_addr.insert(std::make_pair(ip_port_id, task_content.remote_addr_list[index]));
		}
	}
	return true;
}
bool StreamManager::_do_add_ts_task(const TASKCONTENT &task_content, const std::string &local_addr /*= ""*/)
{
	TESTReceiverPtr receiver_ptr = nullptr;
	StreamSenderPtr sender_ptr = nullptr;
	StreamContent stream_content;
	v_lock(lk, stream_map_mutex_);
	if (0 >= strlen(task_content.url))
	{
		return false;
	}
	StreamMap::iterator iter = stream_map_.find(task_content.url);
	if (iter == stream_map_.end())//没有此视频流
	{
		receiver_ptr = std::make_shared<TESTReceiver>(task_content.url);
		if (0 < task_content.addr_cout)
		{
			if (!local_addr.empty())
			{
				sender_ptr = std::make_shared<StreamSender>(receiver_ptr, local_addr);
				sender_ptr->set_local_ip(local_addr);
			}
			else
			{
				sender_ptr = std::make_shared<StreamSender>(receiver_ptr);
			}
			sender_ptr->set_delay_time(task_content.delay_time_ms);
			sender_ptr->set_receive_url(task_content.url);
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
				stream_content.ts_receiver = receiver_ptr;
				stream_map_.insert(std::make_pair(task_content.url, stream_content));
			}
		}
		else//没有转发组播IP
		{
			return false;
		}
	}
	else
	{
		sender_ptr = iter->second.sender;
		for (int index = 0; index < task_content.addr_cout; index++)
		{
			sender_ptr->add_sender_address(task_content.remote_addr_list[index].ip\
				, task_content.remote_addr_list[index].port);
			char ip_port_id[100] = { 0 };
			sprintf(ip_port_id, "%s:%d", task_content.remote_addr_list[index].ip\
				, task_content.remote_addr_list[index].port);
			iter->second.multi_addr.insert(std::make_pair(ip_port_id, task_content.remote_addr_list[index]));
		}
	}
	return true;
}

bool StreamManager::_do_remove_task(const TASKCONTENT &task_content)
{
	v_lock(lk, stream_map_mutex_);
	StreamMap::iterator iter = stream_map_.find(task_content.url);
	if (iter != stream_map_.end())
	{
		int multi_ip_count = iter->second.multi_addr.size();
		for (int task_index = 0; task_index < task_content.addr_cout; task_index++)
		{
			iter->second.sender->del_sender_address(\
				task_content.remote_addr_list[task_index].ip\
				, task_content.remote_addr_list[task_index].port);

			char ip_port_id[100] = { 0 };
			sprintf(ip_port_id, "%s:%d", task_content.remote_addr_list[task_index].ip\
			, task_content.remote_addr_list[task_index].port);
			iter->second.multi_addr.erase(ip_port_id);//删除流的多播地址
		}
		if (task_content.addr_cout == multi_ip_count)//无转发任务
		{
			iter->second.sender->stop();

			iter->second.receiver->stop();
			iter->second.receiver->unsubcribe_ts_callback();

			stream_map_.erase(task_content.url);//从流表中删除
		}
	}
	return true;
}
