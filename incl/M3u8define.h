#pragma once
#include <string>
#include <map>
#include <vector>
using namespace std;

const std::string M3U8_EXT_X_TARGETDURAION = "#EXT-X-TARGETDURATION";
const std::string M3U8_EXT_X_MEDIDA_SEQUENCE = "#MEDIDA-SEQUENCE";
const std::string M3U8_EXT_X_PROGRAM_DATE_TIME = "#EXT-X-PROGRAM-DATE-TIME";
const std::string M3U8_EXT_X_MEDIA = "#EXT-X-MEDIA";
const std::string M3U8_EXT_X_PLAYLIST_TYPE = "#EXT-X-PLAYLIST-TYPE";
const std::string M3U8_EXT_X_KEY = "#EXT-X-KEY";
const std::string M3U8_EXT_X_STREAM_INF = "#EXT-X-STREAM-INF";
const std::string M3U8_EXT_X_VERSION ="#EXT-X-VERSION";
const std::string M3U8_EXT_X_ALLOW_CACHE ="#EXT-X-ALLOW-CACHE";
const std::string M3U8_EXT_X_ENDLIST ="#EXT-X-ENDLIST";
const std::string M3U8_EXT_X_BYTERANGE ="#EXT-X-BYTERANGE";
const std::string M3U8_EXT_X_I_FRAME_STREAM_INF ="#EXT-X-I-FRAME-STREAM-INF";
const std::string M3U8_EXT_X_DISCONTINUITY ="#EXT-X-DISCONTINUITY";
const std::string M3U8_EXT_X_CUE_OUT ="#EXT-X-CUE_OUT-CONT";
const std::string M3U8_EXT_X_SCTE35 ="#EXT-X-SCTE35";
const std::string M3U8_EXT_X_CUE_START ="#EXT-X-CUE-OUT";
const std::string M3U8_EXT_X_CUE_END ="#EXT-X-CUE-IN";
const std::string M3U8_EXTINF = "#EXTINF";
const std::string M3U8_EXT_I_FRAME_ONLY = "#EXT-X-I-FRAMES-ONLY";
const std::string M3U8_EXT_IS_INDEPENDENT_SEGMENTS = "#EXT-X-INDEPENDENT-SEGMENTS";


typedef struct M3U8Struct
{
	int version;
	int current_seq;
	double max_duration;
	//std::map<std::string, double > ts_file_list;/*<< file_name:time*/
	std::vector<std::string> ts_file_list;/*<< file_name:time*/
	M3U8Struct()
		:version(3), current_seq(0), max_duration(0)
	{

	}
}M3U8Data;
