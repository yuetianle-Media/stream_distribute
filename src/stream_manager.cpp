#include "stdafx.h"
#include "stream_manager.h"


StreamManager::StreamManager(const string &config_file)
{
}

StreamManager::~StreamManager()
{
}

bool StreamManager::start()
{
	return false;
}

bool StreamManager::stop()
{
	return false;
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
	}
	if (receiver_ptr)
		receiver_ptr->start();
	if (sender_ptr)
		sender_ptr->start();
	if (receiver_ptr && sender_ptr)
	{
		StreamContent stream_content;
		stream_content.sender = sender_ptr;
		stream_content.receiver = receiver_ptr;
		stream_map_.insert(std::make_pair(task_content.url, stream_content));
	}
	return true;
}

bool StreamManager::_do_remove_task(const TASKCONTENT &task_content)
{
	return true;
}
