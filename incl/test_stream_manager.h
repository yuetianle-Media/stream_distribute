#pragma once
#include "stream_manager.h"
inline void test_stream_manager(const string &config_file)
{
	string stream_rules_file;
	if (config_file.empty())
		stream_rules_file = "rules.xml";
	else
		stream_rules_file = config_file;
	StreamManager manager(stream_rules_file);
	manager.start();
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}
}
