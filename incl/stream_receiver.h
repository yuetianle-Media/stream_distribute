/* Copyright(C)
 * For free
 * All right reserved
 *
 */

#pragma once
#include "pre_std_basic.h"
#include "tcpclient.h"

using namespace std;

typedef enum HttpCMD
{
	HttpM3u8 = 0,
	HttpTs = 1,
}VHttp_t;


//typedef boost::signals2::signal<void(char *, const int&)> M3u8Signal;
//typedef boost::signals2::signal<void(char *, const int&)> TsSignal;

class StreamReceiver
{
public:
	StreamReceiver(const string &url);
	~StreamReceiver();

	int start();
	void stop();

public:
	void subcribe_m3u8_callback();
	void subcribe_ts_callback();
private:
	TCPClientPtr m3u8_tcp_client_ptr_;
	TCPClientPtr ts_tcp_Client_ptr;

	std::string _make_m3u8_cmd();
	std::string _make_down_ts_cmd();

	
	int _send_m3u8_cmd(const string &m3u8_cmd);
	int _send_ts_cmd(const string &ts_cmd);

	void m3u8Callback(char *data, const int &data_len);
	void tsCallback(char *data, const int &data_len);

	int _do_m3u8_task();
	int _do_ts_task();

	std::thread m3u8_thrd_;
	std::thread ts_thrd_;
};

typedef std::shared_ptr<StreamReceiver> StreamReceiverPtr;
