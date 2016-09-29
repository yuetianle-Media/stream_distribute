#include "stdafx.h"
#include "test_rule_manager.h"
void test_rule_manager(const std::string &config_file)
{
	RuleManager rule_manager(config_file);
	//consume thread;
	thread thread1([&rule_manager]()
	{
		TASKCONTENT task_content;
		TASKCONTENT del_task_content;
		while (1)
		{
			while (rule_manager.get_task(task_content))
			{
				std::cout << "add url:" << task_content.url << std::endl;
				std::cout << "ip list:" << std::endl;
				int index = 0;
				
				for (int index =0; index < task_content.addr_cout; index++)
				{
					std::cout << "ip:" << task_content.remote_addr_list[index].ip\
						<< "port:" << task_content.remote_addr_list[index].port << std::endl;
				}
				//consume_task(task_content);

			}
			while (rule_manager.get_del_task(del_task_content))
			{
				std::cout << "del url:" << del_task_content.url << std::endl;
				std::cout << "ip list:" << std::endl;
				int index = 0;
				
				for (int index =0; index < del_task_content.addr_cout; index++)
				{
					std::cout << "ip:" << del_task_content.remote_addr_list[index].ip\
						<< "port:" << del_task_content.remote_addr_list[index].port << std::endl;
				}
				//consume_task(task_content);
			}

			this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	);
	while (1)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}

}