#include "stdafx.h"
#include "ts_file_stream_receiver.h"
#include "errcode.h"


/*TSFileStreamReceiver::TSFileStreamReceiver(const char * file_name)
	:ts_file_name_((char*)file_name), BaseReceiver()
{

}
TSFileStreamReceiver::~TSFileStreamReceiver()
{

}


int TSFileStreamReceiver::start()
{
	if (ts_file_name_)
	{
		boost::filesystem::path ts_file_path(ts_file_name_);
		if (!boost::filesystem::exists(ts_file_path))
		{
			return E_FILE_EMPTY;
		}
		FILE* ts_fd = fopen(ts_file_name_, "rb");
		if (!ts_fd)
			return E_FILE_ERROR;
		while (!feof(ts_fd))
		{
			fread()
			//
		};
	}
	return 0;
}

int TSFileStreamReceiver::stop()
{
	return 0;
}

bool TSFileStreamReceiver::get_ts_send_data_queue(TSSendSpscQueueType *& send_queue)
{
	return 0;
}
*/