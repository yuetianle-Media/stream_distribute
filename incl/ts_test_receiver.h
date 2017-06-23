/**
 * @file stream_receiver.h
 * @brief 数据接收类.
 * @author lee, shida23577@hotmail.com
 * @version 1.0.1
 * @date 2016-09-27
 */

#ifndef _TS_RECEIVER_H_
#define _TS_RECEIVER_H_
#pragma once

#include "pre_boost_basic.h"
#include "pugixml.hpp"

#include "stream_buffer.h"

#include "tspacket.h"
#include "stream_sender_buffer.h"

#include "tcpclient.h"
#include "uri_parser.h"
#include "M3u8Parser.h"
#include "http_curl_client.h"
#include "thread_pool.h"
using namespace pugi;
#ifdef __linux
#include <error.h>
#endif
#include "data_types_defs.h"

#define ENABLE_OUT_PCR 0 
class TESTReceiver :public std::enable_shared_from_this<TESTReceiver> //: boost::signals2::trackable
{
public:
	TESTReceiver(const std::string &url);
	~TESTReceiver();

	int start();
	void stop();
	//bool get_send_queue(TSSendQueueType *&send_queue) { send_queue = &ts_send_queue_; return true; }
	bool get_send_queue(TSSendSpscQueueType *&send_queue) { send_queue = &ts_send_queue_; return true; }
	bool get_ts_send_sd_queue(TSSendSpscSDQueueTypePtr &ts_send_queue);
	bool get_ts_send_hd_queue(TSSendSpscHDQueueTypePtr &ts_send_queue);
	bool get_ts_send_fhd_queue(TSSendSpscFHDQueueTypePtr &ts_send_queue);
	bool get_ts_send_2k_queue(TSSendSpsc2KQueueTypePtr &ts_send_queue);
	bool get_ts_send_4k_queue(TSSendSpsc4KQueueTypePtr &ts_send_queue);
	//bool get_unlimit_queue(TSSendUnlimitQueueType *&unlimit_queue) { unlimit_queue = &ts_send_unmlimt_queue_; return true; }
public:
    /**
     * @brief subcribe_ts_callback 订阅ts流数据.
     *
     * @param slot 数据流callback.
     *
     * @returns 订阅连接.
     */
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


	int _push_ts_cmd_to_queue(const string &ts_cmd);

	/*
	 * 删除当前time_second之前的任务
	*/

	int _do_ts_task_group();

	int _do_ts_task();

	int _push_ts_data_to_queue(string &ts_data);

	int tsCallback(char *data, const long int &data_len, const bool&is_finished);

	void _do_parse_ts_data();

	template <typename T>
	void _doing_push_ts_data_to_queue(T &data_queue, TSSENDCONTENT &ts_content);
	void _do_parse_ts_data_ext();

	void _push_ts_data_to_send_queue(char *data, const long int &data_len, const int &need_time/*bytes:(188*7):micro*/);

	void _write_ts_file_list(const string &out_file_name, const string &ts_file_name, const int &index, const string &time="");

	void _write_content_to_file(const string &out_file_name, char *data, const int &data_len);
private:

	pugi::xml_document ts_file_doc_;
	std::string ts_task_file_name_;/*<< url+time*/

    std::string play_stream_;/* << http cmd to play a stream*/
    int play_stream_duration_;/*<< duration to send http cmd*/

	long int play_stream_count_;
	std::map<int, std::string> play_stream_list_;/*<< when the play_stream is variant play lists*/

	std::map<std::string, std::string> ts_all_task_map_;/*<< task_uri, time*/
    TSTaskType ts_task_list_;

    std::shared_ptr<std::thread> ts_thrd_ptr_;
	std::shared_ptr<std::thread> parse_ts_thrd_;


	long int ts_file_index_;
	atomic<bool> b_exit_ts_task_;/*<< exit the ts task thread */
	atomic<bool> b_exit_parse_task_;/*<< exit the parse ts task thread*/

	std::string uri_;

	std::string m3u8_content_;

	StreamBuffer receive_ts_buffer_;
	TSPacketSpscQueueType ts_spsc_packet_queue;

	TSSendSpscQueueType ts_send_queue_;
	std::shared_ptr<boost::asio::io_service> io_svt_;
	std::shared_ptr<boost::asio::io_service::work> worker_;
	boost::asio::io_service::strand strand_;

	std::condition_variable ts_data_condition_;
	std::once_flag ts_data_flag_;
	std::string cur_date_;
	std::string unique_str_;

	//TS 数据缓冲区
	enum StreamRate
	{
		RateSD = 0,
		RateHD = 1,
		RateFHD = 2,
		Rate2K = 3,
		Rate4K = 4
	};
	int stream_rate_;
	ts_comm_send_queue ts_comm_send_queue_;
};

template <typename T>
void TESTReceiver::_doing_push_ts_data_to_queue(T &data_queue, TSSENDCONTENT &ts_content)
{
	while (1 > data_queue->write_available())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (!data_queue->push(ts_content))
	{
		std::cout << "push ts data faile" << std::endl;
	}
}

typedef std::shared_ptr<TESTReceiver> TESTReceiverPtr;
#endif
