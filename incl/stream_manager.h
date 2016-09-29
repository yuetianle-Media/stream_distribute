#ifndef _STREAM_MANAGER_H_
#define _STREAM_MANAGER_H_
#pragma once
#include "stream_receiver.h"
#include "stream_sender.h"
#include "rule_manager.h"

typedef std::map<std::string, StreamReceiverPtr> ReceiverMap;/*<< url:receiver*/
typedef std::map<std::string, StreamSenderPtr>	 SenderMap;/*<< url:sender*/
typedef struct MStreamContent
{
	StreamReceiverPtr receiver;
	StreamSenderPtr sender;
	MStreamContent()
		:receiver(nullptr), sender(nullptr)
	{
		
	}
}StreamContent;
typedef std::map<std::string, StreamContent> StreamMap;/*<< url:receiver+sender*/
class StreamManager
{
public:
	StreamManager(const string &config_file);
	~StreamManager();

	bool start();
	bool stop();
private:

	void _do_add_tasks_callback();
	void _do_del_tasks_callback();

	bool _do_add_task(const TASKCONTENT &task_content);
	bool _do_remove_task(const TASKCONTENT &task_content);

	std::shared_ptr<std::thread> add_task_;
	std::shared_ptr<std::thread> remove_task_;
	string rule_config_file_;
	RuleManagerPtr rules_manager_;
	ReceiverMap receiver_;
	SenderMap sender_;
	StreamMap stream_map_;
	std::atomic<bool> is_exit_;
};
#endif
