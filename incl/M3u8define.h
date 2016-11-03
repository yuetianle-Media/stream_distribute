#pragma once
#include <string>
#include <map>
#include <tuple>
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


const std::string EXT_STREAM_INFO_PROGRAM_ID_NAME = "PROGRAM-ID";
const std::string EXT_STREAM_INFO_BANDWIDTH_NAME = "BANDWIDTH";
const std::string EXT_STREAM_INFO_AVERAGE_BANDWIDTH_NAME = "AVERAGE-BANDWIDTH";
const std::string EXT_STREAM_INFO_RESOLUTION_NAME = "RESOLUTION";
const std::string EXT_STREAM_INFO_CODEC_NAME = "CODEC";
const std::string EXT_STREAM_INFO_VIDEO_NAME = "VIDEO";
const std::string EXT_STREAM_INFO_AUDIO_NAME = "AUDIO";
const std::string EXT_STREAM_INFO_SUBTITLES_NAME = "SUBTITLES";
/*

band_width
average_band_width
program_id
resolution
codec
video
audio
subtitles
examples:
#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=800000,AVERAGE-BANDWIDTH=7560423,RESOLUTION=624x352,CODECS="avc1.4d001f, mp4a.40.5
*/
using StreamInfoExt = std::tuple<long int, long int, long int\
	, std::string, std::string, std::string\
	, std::string, std::string>;

using StreamInfo = struct Stream_Info 
{
	long int band_width;
	long int average_band_width;
	long int program_id;
	std::string resolution;
	std::string codec;
	std::string video;
	std::string audio;
	std::string subtitles;
	Stream_Info()
		:band_width(0), average_band_width(0), program_id(-1)
	{

	}
};
using StreamInfoList = std::vector<StreamInfo>;

using MediaInfoExt = std::map<std::string, std::string>;
using MediaInfo = struct Media_Info
{
	std::string uri;
	std::string type;
	std::string group_id;
	std::string language;
	std::string name;
	bool is_default;
	bool auto_selected;
	bool forced;
	std::string assoc_language;
	std::string instream_id;
	std::string characteristics;
	Media_Info()
		:is_default(true), auto_selected(true), forced(false)
	{

	}
};
using MediaInfoList = std::vector<MediaInfo>;

using PlayList = struct Play_List
{
	std::string uri;

	std::string base_uri;

	MediaInfo media_info;
	StreamInfo stream_info;
};
using PlayListList = std::vector<PlayList>;

using M3U8Segment = struct M3u8VideoSegment
{
	std::string uri;
	std::string title;/*<< title attribute from EXTINF parameter*/
	std::string program_data_time;/*<< EXT-X-PROGRAM-DATE-TIME*/
	bool discontinuity;/*<< EXT-X-DISCONTINUITY*/
	bool cue_out;/*<< EXT-X-CUE-OUT_CONT*/
	std::string scte35;
	std::string scte35_duration;
	float duration;/*<< duration attribute from EXTINF parameter*/
	std::string base_uri;
	long int video_size;/*<< byterange attribute from EXT-X-BYTERANGE paramter*/
	long int video_data_offset;/*<< byterange attribute from EXT-X-BYTERANGE paramter*/
	std::string key;/*<< key used to encrypt the segment (EXT-X-KEY)*/
	M3u8VideoSegment()
		:discontinuity(false), cue_out(false), duration(0.0f)\
		, video_size(0), video_data_offset(0)
	{
	}
};
using M3U8SegmentList = std::vector<M3U8Segment>;

using M3U8Key = struct M3U8_Key
{
	std::string method;
	std::string uri;
	std::string base_uri;
	std::string iv;
	std::string key_format;
	std::string key_format_versions;
};

enum class PlayListType {
	VOD, EVENT
};

using M3U8State = struct M3U8_State{
	bool expect_segments;
	bool expect_playlist;
	M3U8_State()
		:expect_playlist(false), expect_segments(false)
	{}
};
using M3U8Data = struct M3U8Struct
{
	std::string base_uri;
	std::string uri;
	M3U8Key keys;/*<< EXT-X-KEY*/
	/*
	Returns true if this M3U8 is a variant playlist, with links to
	other M3U8s with different bitrates.
	If true, `playlists` is a list of the playlists available,
	and `iframe_playlists` is a list of the i-frame playlists available.
	*/
	bool is_variant;
	/*
	Returns true if EXT - X - ENDLIST tag present in M3U8.
	*/
	bool is_end_list;
	/*
	If this is a variant playlist(`is_variant` is True), returns a list of
	Playlist objects.
	*/
	PlayListList play_lists;
	/*
        A lower-case string representing the type of the playlist, which can be
        one of VOD (video on demand) or EVENT.
	*/
	PlayListType play_list_type;

	/*
	 If this is a variant playlist (`is_variant` is True), returns a list of
	Media objects.
	*/
	MediaInfoList media_info_lists;

	/*
	Returns the EXT-X-TARGETDURATION as an integer
	http://tools.ietf.org/html/draft-pantos-http-live-streaming-07#section-3.3.2.
	*/
	double target_duration;

	/*
	 Returns the EXT-X-MEDIA-SEQUENCE as an integer
	http://tools.ietf.org/html/draft-pantos-http-live-streaming-07#section-3.3.3.
	*/
	long int media_sequence;
	/*
        Returns the EXT-X-PROGRAM-DATE-TIME as a string
        http://tools.ietf.org/html/draft-pantos-http-live-streaming-07#section-3.3.5.
	*/
	std::string program_data_time;
	/*
       a `SegmentList` object, represents the list of `Segment`s from this playlist
	*/
	M3U8SegmentList segments;

	/*
        Return the EXT-X-VERSION as is
	*/
	int version;
	/*
        Return the EXT-X-ALLOW-CACHE as is
	*/
	bool allow_cache;
	/*
        Returns an iterable with all files from playlist, in order. This includes
        segments and key uri, if present.
	*/
	/*
        Returns true if EXT-X-I-FRAMES-ONLY tag present in M3U8.
        http://tools.ietf.org/html/draft-pantos-http-live-streaming-07#section-3.3.12
	*/
	bool is_i_frames_only;
	/*
        Returns true if EXT-X-INDEPENDENT-SEGMENTS tag present in M3U8.
        https://tools.ietf.org/html/draft-pantos-http-live-streaming-13#section-3.4.16
	*/
	bool is_independent_segments;
	std::vector<std::string> files;
	//std::map<std::string, double > ts_file_list;/*<< file_name:time*/
	std::vector<std::string> ts_file_list;/*<< file_name:time*/
	M3U8Struct()
		:is_variant(false), is_end_list(false), version(3)\
		, media_sequence(0), target_duration(0.0), is_i_frames_only(false)\
		, is_independent_segments(false), allow_cache(false)
	{

	}
};
