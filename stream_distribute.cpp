// stream_distribute.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_rule_manager.h"
#include "test_stream_receiver.h"

int main()
{
	const string config_file = "rules.xml";
	test_rule_manager(config_file);
	while (true)
	{
		this_thread::sleep_for(std::chrono::seconds(5));
	}
    return 0;
}

