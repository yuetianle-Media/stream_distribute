
#pragma once
#include "udpclient.h"
#include "pre_boost_basic.h"
#include <boost/asio.hpp>
//#include "Udp.h"

using namespace std;
using namespace boost::asio::ip;
void test_udp_sync_client();

void test_udp_client(const string &addr, const int &port);
