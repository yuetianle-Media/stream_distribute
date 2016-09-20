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

typedef struct M3U8Struct
{
	int version;
	int current_seq;
	double max_duration;
	std::map<std::string, double > ts_file_list;/*<< file_name:time*/
	M3U8Struct()
		:version(3), current_seq(0), max_duration(0)
	{

	}
}M3U8Data;

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

	//int _parser_m3u8_file(const char *m3u8_data, const int &length);
	M3U8Data _parser_m3u8_file(const char *m3u8_data, const int &length);

	int _push_ts_cmd(const string &ts_cmd);
	bool _get_ts_cmd(TSCMD &cmd);

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
	std::map<std::string, int> ts_all_task_map_;/*<< task_uri, task_index*/
    //boost::lockfree::queue<std::string> ts_task_list_;
    boost::lockfree::queue<TSCMD, boost::lockfree::fixed_sized<true>> ts_task_list_;

	//boost::shared_ptr<boost::thread> m3u8_thrd_ptr_;
    //boost::shared_ptr<boost::thread> ts_thrd_ptr_;
    std::shared_ptr<std::thread> m3u8_thrd_ptr_;
    std::shared_ptr<std::thread> ts_thrd_ptr_;

	URIParser uri_parser_;

	int ts_file_index_;
    bool b_exit;/* << exit the thread.*/
	std::string http_packet;/*<< one http packet*/
};

typedef std::shared_ptr<StreamReceiver> StreamReceiverPtr;
