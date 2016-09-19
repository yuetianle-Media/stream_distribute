/* Copyright(C)
 * For free
 * All right reserved
 *
 */

#pragma once

#include <boost/lockfree/queue.hpp>
#include <boost/filesystem.hpp>
#include "tcpclient.h"
#include "uri_parser.h"

//using namespace std;

typedef enum HttpCMD
{
	HttpM3u8 = 0,
	HttpTs = 1,
}VHttp_t;

struct TSCMD
{
	char cmd[1024];
	int cmd_length;
	TSCMD()
	{
		memset(cmd, 0, sizeof(TSCMD));
	}
};


typedef boost::signals2::signal<void(char *, const int&)> M3u8Signal;
typedef boost::signals2::signal<void(char *, const int&)> TsSignal;

class StreamReceiver
{
public:
	StreamReceiver(const std::string &url);
	~StreamReceiver();

	int start();
	void stop();

public:
    //void subcribe_m3u8_callback(const M3u8Signal::slot_type &slot);
    boost::signals2::connection subcribe_ts_callback(const TsSignal::slot_type &slot);
private:
	TCPClientPtr m3u8_tcp_client_ptr_;
	TCPClientPtr ts_tcp_client_ptr_;

	std::string _make_m3u8_cmd(const std::string &stream_url="");
	std::string _make_down_ts_cmd(const std::string &ts_file);


	int _send_m3u8_cmd(const std::string &m3u8_cmd);
	int _send_ts_cmd(const std::string &ts_cmd);

	void m3u8Callback(char *data, const int &data_len);
	void tsCallback(char *data, const int &data_len);

	int _do_m3u8_task();
	int _do_ts_task();

    TsSignal ts_send_signal_;

    boost::signals2::connection m3u8_conn;/* << m3u8 data callback connection*/
    boost::signals2::connection ts_conn;/* << ts data callback connection*/
    std::string stream_url_;/* << uri to play a stream.*/
    std::string play_stream_;/* << http cmd to play a stream*/
    int play_stream_duration_;/*<< duration to send http cmd*/

    vector<std::string> ts_file_list_;
    //boost::lockfree::queue<std::string> ts_task_list_;
    boost::lockfree::queue<TSCMD, boost::lockfree::fixed_sized<false>> ts_task_list_;

	//boost::shared_ptr<boost::thread> m3u8_thrd_ptr_;
    //boost::shared_ptr<boost::thread> ts_thrd_ptr_;
    std::shared_ptr<std::thread> m3u8_thrd_ptr_;
    std::shared_ptr<std::thread> ts_thrd_ptr_;

	URIParser uri_parser_;

    bool b_exit;/* << exit the thread.*/
};

typedef std::shared_ptr<StreamReceiver> StreamReceiverPtr;
