#pragma once
#include "stream_receiver.h"
#include "stream_sender.h"
inline void test_stream_sender(const string &url, const string &multi_server, const int &port)
{
    int test_count = 1;
    std::map<int, boost::signals2::connection> conn_list;
    std::map<int, std::shared_ptr<StreamReceiver>> receive_list;
	std::map<int, StreamReceiver> receive_list_ext;
    std::map<int, std::shared_ptr<StreamSender>> sender_list;
    while (0 < test_count)
    {
		std::shared_ptr<StreamReceiver> ptr = nullptr;
		std::shared_ptr<StreamSender> ptr_send = nullptr;
		ptr = std::make_shared<StreamReceiver>(url);
		ptr_send = std::make_shared<StreamSender>();
		std::cout << "receive count:" << ptr.use_count() << "send count:" << ptr_send.use_count();
        ptr_send->add_sender_address(multi_server, port);
		//receive_list_ext.insert(std::make_pair(test_count, stream_receiver));
        //boost::signals2::connection sender_connect = stream_receiver.subcribe_ts_callback(boost::bind(&StreamSender::stream_receive_callback, &stream_sender, _1,_2));
        //boost::signals2::connection sender_connect = ptr->subcribe_ts_callback(boost::bind(&StreamSender::stream_receive_callback, &stream_sender, _1,_2));
        boost::signals2::connection sender_connect = ptr->subcribe_ts_callback(boost::bind(&StreamSender::stream_receive_callback, ptr_send, _1,_2, _3));
        ptr->start();
        ptr_send->start();
        conn_list.insert(std::make_pair(test_count, sender_connect));
        receive_list.insert(std::make_pair(test_count, ptr));
        sender_list.insert(std::make_pair(test_count, ptr_send));
		this_thread::sleep_for(std::chrono::seconds(5));
        test_count--;
    }
	while (1)
	{
        for (auto sender_connect:conn_list)
        {
            if (!sender_connect.second.connected())
            {
                std::map<int, std::shared_ptr<StreamReceiver>>::iterator iter = receive_list.find(sender_connect.first);
                std::map<int, std::shared_ptr<StreamSender>>::iterator iter_send = sender_list.find(sender_connect.first);
                if (iter != receive_list.end() && iter_send != sender_list.end())
                {
                    auto new_conn = iter->second->subcribe_ts_callback(boost::bind(&StreamSender::stream_receive_callback\
                                , iter_send->second, _1,_2,_3));
                    sender_connect.second = new_conn;
                }
            }
            else {
                continue;
            }
        }
		this_thread::sleep_for(std::chrono::seconds(5));
	}
};
