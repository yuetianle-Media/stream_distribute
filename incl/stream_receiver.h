#ifndef _STREAM_RECEIVER_H_
#define _STREAM_RECEIVER_H_
#pragma once

#include <boost/lockfree/queue.hpp>
#include <boost/filesystem.hpp>
#include "pugixml.hpp"

#include "tcpclient.h"
#include "uri_parser.h"
#include "m3u8Parser.h"

using namespace pugi;

typedef boost::signals2::signal<void(char *, const int&)> M3u8Signal;
typedef boost::signals2::signal<void(char *, const long int&)> TsSignal;

class StreamReceiver
{
public:
	StreamReceiver(const std::string &url);
	~StreamReceiver();

	int start();
	void stop();
public:
    boost::signals2::connection subcribe_ts_callback(const TsSignal::slot_type &slot);

private:
	std::string _make_m3u8_cmd(const std::string &stream_url="");
	bool _make_m3u8_cmd(HTTPM3U8CMD &m3u8_cmd, const std::string &stream_url="");
	std::string _make_down_ts_cmd(const std::string &ts_file);
	bool _make_down_ts_cmd(HTTPTSCMD &ts_cmd, const std::string &ts_file);


	int _send_m3u8_cmd(const std::string &m3u8_cmd);
	int _send_ts_cmd(const std::string &ts_cmd);

	bool _parser_m3u8_file(const char *m3u8_data, const int &length, M3U8Data &m3u8_data_struct);

	int _push_ts_cmd(const string &ts_cmd);
	bool _get_ts_cmd(HTTPTSCMD &cmd);

	void m3u8Callback(char *data, const int &data_len);
	void tsCallback(char *data, const int &data_len);

	int _do_m3u8_task();
	int _do_ts_task();

	void _write_ts_file_list(const string &ts_file_name, const int &index);
	void _write_content_to_file(char *data, const int &data_len);
private:
	TCPClientPtr m3u8_tcp_client_ptr_;
	TCPClientPtr ts_tcp_client_ptr_;
	pugi::xml_document ts_file_doc_;

    TsSignal ts_send_signal_;

    boost::signals2::connection m3u8_conn;/* << m3u8 data callback connection*/
    boost::signals2::connection ts_conn;/* << ts data callback connection*/
    std::string stream_url_;/* << uri to play a stream.*/
    std::string play_stream_;/* << http cmd to play a stream*/
    int play_stream_duration_;/*<< duration to send http cmd*/

    vector<std::string> ts_file_list_;
	std::map<std::string, int> ts_all_task_map_;/*<< task_uri, task_index*/
    boost::lockfree::queue<HTTPTSCMD, boost::lockfree::fixed_sized<true>> ts_task_list_;

    std::shared_ptr<std::thread> m3u8_thrd_ptr_;
    std::shared_ptr<std::thread> ts_thrd_ptr_;

	URIParser uri_parser_;

	int ts_file_index_;
    bool b_exit;/* << exit the thread.*/
	std::string http_packet;/*<< one http packet buff with m3u8 data*/
	std::string http_ts_packet_;/*<< one http packet buff with ts data*/
	int save_content_index_;
};

typedef std::shared_ptr<StreamReceiver> StreamReceiverPtr;
#endif
