/**
 * @file stream_receiver.h
 * @brief 数据接收类.
 * @author lee, shida23577@hotmail.com
 * @version 1.0.1
 * @date 2016-09-27
 */

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
    /**
     * @brief subcribe_ts_callback 订阅ts流数据.
     *
     * @param slot 数据流callback.
     *
     * @returns 订阅连接.
     */
    boost::signals2::connection subcribe_ts_callback(const TsSignal::slot_type &slot);


	void tsCallback(char *data, const int &data_len);
private:
	bool _find_http_header_start(char* &dest, char*src, const int src_length);
	bool _find_http_header_end(char* &dest, char*src, const int src_length);

	bool _find_http_line_end(char* &dest, char*src, const int src_length);
    /**
     * @brief _make_m3u8_cmd 创建http的m3u8命令.
     * @example
     * GET /hls/test.m3u8 HTTP/1.1
     *
     * @param stream_url hls流地址 http://192.168.1.1:8000/hls/test.m3u8.
     *
     * @returns
     */
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

	//void tsCallback(char *data, const int &data_len);

	int _do_m3u8_task();

	int _do_ts_task();

	void _write_ts_file_list(const string &out_file_name, const string &ts_file_name, const int &index);


	void _write_content_to_file(const string &out_file_name, char *data, const int &data_len);
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
	std::atomic<int> callback_times_;
    bool b_exit;/* << exit the thread.*/
	std::string http_packet;/*<< one http packet buff with m3u8 data*/
	std::string http_ts_packet_;/*<< one http packet buff with ts data*/
	int save_content_index_;
	std::atomic<bool> is_receive_ts_response_header_;
	long int ts_response_content_length_;
};

typedef std::shared_ptr<StreamReceiver> StreamReceiverPtr;
#endif
