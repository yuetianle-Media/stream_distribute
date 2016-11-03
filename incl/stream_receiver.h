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

#include "stream_buffer.h"
#include "response.h"
#include "httpresponseparser.h"

#include "tspacket.h"
#include "stream_sender_buffer.h"

#include "tcpclient.h"
#include "uri_parser.h"
#include "m3u8Parser.h"
#include "http_curl_client.h"
using namespace pugi;
#ifdef __linux
#include <error.h>
#endif
#include "data_types_defs.h"

class StreamReceiver :public std::enable_shared_from_this<StreamReceiver> //: boost::signals2::trackable
{
public:
	StreamReceiver(const std::string &url);
	~StreamReceiver();

	int start();
	void stop();
	bool get_send_queue(TSSendQueueType *&send_queue) { send_queue = &ts_send_content_queue_; return true; }
public:
    /**
     * @brief subcribe_ts_callback 订阅ts流数据.
     *
     * @param slot 数据流callback.
     *
     * @returns 订阅连接.
     */
    boost::signals2::connection subcribe_ts_callback(const TsSignal::slot_type &slot);

	void unsubcribe_ts_callback();
private:
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

	int _do_m3u8_task();

	int _do_m3u8_task_group(const string &play_stream, const long int play_duration/*unit:s*/);

	int _send_m3u8_cmd(const std::string &m3u8_cmd);

	void m3u8Callback(char *data, const long int &data_len, const bool &is_finished);

	bool _parser_m3u8_file(const char *m3u8_data, const int &length, M3U8Data &m3u8_data_struct);

	std::string _make_down_ts_cmd(const std::string &ts_file);

	bool _make_down_ts_cmd(HTTPTSCMD &ts_cmd, const std::string &ts_file);

	int _push_ts_cmd_to_queue(const string &ts_cmd);

	/*
	 * 删除当前time_second之前的任务
	*/
	int _remove_ts_cmd(const long int time_second);

	int _do_ts_task();

	int _send_ts_cmd(const std::string &ts_cmd);

	void tsCallback(char *data, const long int &data_len, const bool&is_finished);

	void _do_parse_ts_data();

	void _push_ts_data_to_send_queue(char *data, const long int &data_len, const int &need_time/*bytes:(188*7):micro*/);

	void _write_ts_file_list(const string &out_file_name, const string &ts_file_name, const int &index);

	void _write_content_to_file(const string &out_file_name, char *data, const int &data_len);
private:

	pugi::xml_document ts_file_doc_;
	std::string ts_task_file_name_;/*<< url+time*/

    TsSignal ts_send_signal_;

    boost::signals2::connection m3u8_conn_;/* << m3u8 data callback connection*/

    boost::signals2::connection ts_conn_;/* << ts data callback connection*/

    std::string play_stream_;/* << http cmd to play a stream*/
    int play_stream_duration_;/*<< duration to send http cmd*/

	long int play_stream_count_;
	std::map<int, std::string> play_stream_list_;/*<< when the play_stream is variant play lists*/

	std::map<std::string, std::string> ts_all_task_map_;/*<< task_uri, time*/
    TSTaskType ts_task_list_;

    std::shared_ptr<std::thread> m3u8_thrd_ptr_;
    std::shared_ptr<std::thread> ts_thrd_ptr_;
	std::shared_ptr<std::thread> parse_ts_thrd_;

	URIParser uri_parser_;

	long int ts_file_index_;
    atomic<bool> b_exit_m3u8_task_;/* << exit the m3u8 task thread.*/
	atomic<bool> b_exit_ts_task_;/*<< exit the ts task thread */
	atomic<bool> b_exit_parse_task_;/*<< exit the parse ts task thread*/

	HTTP_CURL_CLIENT_PTR m3u8_http_client_ptr_;
	HTTP_CURL_CLIENT_PTR ts_http_client_ptr_;
	std::string uri_;

	std::string m3u8_content_;

	StreamBuffer receive_ts_buffer_;
	TSPacketQueueType ts_packet_queue_;//32K一级数据缓存大小
	TSSendQueueType ts_send_content_queue_;//128k二级缓存大小
	
	TSTaskGroup ts_task_group_;
	std::map<int, int> ts_task_group_2;
	
};

typedef std::shared_ptr<StreamReceiver> StreamReceiverPtr;
#endif
