#include "stdafx.h"
#include "stream_receiver.h"

void test_stream_receiver()
{
	const string stream_url = "http://192.168.203.130:9006/hls/test.m3u8";
	StreamReceiver stream_receiver(stream_url);
	stream_receiver.start();
}