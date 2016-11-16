
#pragma once
#include "udpclient.h"
#include "pre_boost_basic.h"
#include "tspacket.h"
#include "stream_sender_buffer.h"
#include "stream_sender.h"
//#include "Udp.h"
using namespace std;
//using namespace std;
//using namespace boost::asio::ip;
void test_udp_sync_client();

void test_udp_client(const string &addr, const int &port);
