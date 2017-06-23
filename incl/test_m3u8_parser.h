#pragma once
#include "playlist.h"
#include "M3u8Parser.h"
inline void test_m3u8_content_parser()
{
	std::string src_uri_;
	//M3u8Parser parser_(src_uri_, SIMPLE_PLAYLIST);
	M3u8Parser parser_one(src_uri_, PLAYLIST_USING_BYTERANGES);
	M3u8Parser parser_data_time(src_uri_, SIMPLE_PLAYLIST_WITH_PROGRAM_DATE_TIME);
	M3u8Parser parser_title(src_uri_, SIMPLE_PLAYLIST_WITH_TITLE);
	M3u8Parser variand_stream_info(src_uri_, VARIANT_PLAYLIST);
	std::this_thread::sleep_for(std::chrono::seconds(5));
}
