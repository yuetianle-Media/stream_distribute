#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <string.h>
#ifdef __linux__
#include <strings.h>
#endif // linux

#include "pre_boost_basic.h"
#include "pre_regex.h"

#include "M3u8define.h"
#include "errcode.h"
//warning not thread safe.

inline const char * STRSTR(const char *src, const char *substr)
{
#ifdef _WIN32
	return strstr(src, substr);
#else
	int len = strlen(substr);
	if (0 == len)
	{
		return NULL;
	}
	while (*src)
	{
		if (0 == strncasecmp(src, substr, len))
		{
			return src;
		}
		++src;
	}
	return NULL;
#endif // _WIN32
}
inline bool v_str_is_start_with(const std::string &src, const std::string &flag)
{
	//char *index = strstr((char*)src.c_str(), (char*)flag.c_str());
	const char *index = STRSTR((char*)src.c_str(), (char*)flag.c_str());
	if (nullptr != index && 0 == int(index - src.c_str()))
	{
		return true;
	}
	else
	{
		return false;
	}
};

/*
	* remove sep charaters from string 
	* example src "dfjsdlf\"df,\"dfd," dest "dfjsdlf\"df\"dfd"
*/
inline std::string v_str_remove_quotes(const std::string &src, const std::string sep = ",")
{
	std::string dest_str = src;
	int first_pos = dest_str.find("\"");
	std::vector<int> remove_index_list;
	while (string::npos != first_pos)
	{
		int second_pos = dest_str.find("\"", first_pos+1);
		if (string::npos != second_pos)
		{
			int length = second_pos - first_pos;
			int pos=dest_str.find(",", first_pos+1);
			if (std::string::npos != pos && pos < second_pos)
			{
				remove_index_list.push_back(pos);
			}
		}
		first_pos = dest_str.find("\"", second_pos+1);
	}
	for (auto remove_index : remove_index_list)
	{
		dest_str.erase(remove_index, 1);
	}
	return dest_str;
}
inline int v_str_split(const std::string& str, std::vector<std::string>& ret_, std::string sep = ",")
{
	if (str.empty())
	{
		return 0;
	}

	std::string tmp;
	std::string::size_type pos_begin = str.find_first_not_of(sep);
	std::string::size_type comma_pos = 0;

	while (pos_begin != std::string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != std::string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
			pos_begin = comma_pos;
		}

		if (!tmp.empty())
		{
			ret_.push_back(tmp);
			tmp.clear();
		}
	}
	return 0;
}

inline std::string v_str_replace(const std::string& src, const std::string& replacement, const std::string& dest)
{
	std::string ret;
	std::string::size_type pos_begin = 0;
	std::string::size_type pos = src.find(replacement);
	while (pos != std::string::npos)
	{
#ifdef _DEBUG
		//std::cout << "replacexxx:" << pos_begin << " " << pos << std::endl;
#endif
		ret.append(src.data() + pos_begin, pos - pos_begin);
		ret += dest;
		pos_begin = pos + replacement.length();
		pos = src.find(replacement, pos_begin);
	}
	if (pos_begin < src.length())
	{
		ret.append(src.begin() + pos_begin, src.end());
	}
	return ret;
}

union ContentType
{
	std::string normal_data;
	std::vector<std::string> list_data;
};
typedef struct M3u8BaseData
{
	int media_sequence;
	int target_duration;//unit:s
	bool is_variant;
	bool is_endlist;
	bool is_i_frame_only;
	bool is_independent_segments;
	std::string playlist_type;
	std::vector<std::string> playlists;
	std::vector<std::string> iframe_playlists;
	std::vector<std::string> segments;
	std::vector<std::string> media;
	M3u8BaseData()
		:media_sequence(0), is_variant(false)\
		, is_endlist(false), is_i_frame_only(false)\
		, is_independent_segments(false), target_duration(0)
	{

	}
}M3u8NormalData;
class M3u8Parser
{
public:
	//M3u8Parser(const std::string &content);
	M3u8Parser(const std::string uri, const std::string &content);
	~M3u8Parser();
	//static M3u8NormalData s_data;
	//static std::map<std::string, ContentType>s_data;
	typedef std::map<std::string, ContentType>::iterator DATA_ITER;

	bool is_variant_play_list() const { return data_.is_variant; }
	bool get_play_list(PlayListList *&play_list);
	bool get_segments(M3U8SegmentList *&segment_list);
	bool get_ts_file_list(std::vector<std::string> &ts_file_list);
	float get_max_duration();
private:
	//int _parse_key(const std::string &content);
	int _parse_extintf(M3U8Segment *segment, const std::string &content, const int &line_no, bool strict);
	int _parse_ts_chunk(M3U8Segment *segment, const std::string &content);
	//int _parse_attribute_list(const std::string prefix, const std::string &content, void* func);
	int _parse_stream_inf(StreamInfo *stream_info, const std::string &content);
	//int _parse_i_frame_stream_inf(const std::string &content);
	int _parse_media(MediaInfo*media_info, const std::string &content);
	int _parse_variant_playlist(StreamInfo*stream_info, MediaInfo*media_info, const std::string &content);
	int _parse_byterange(M3U8Segment *segment, const std::string &content);
	int _parse_simple_parameter_raw_value(const std::string &content, const std::string &cast_to, bool normalize=false);
	int _parse_and_set_simple_parameter_raw_value(const std::string &content, const std::string &cast_to, bool normalize=false);
	int _parse_simple_parameter(const std::string &content, const std::string &cast_to);
	//int _parse_cueout(const std::string &content);
	int _strings_to_lines(const std::string &content);

	std::vector<std::string> content_lines_;
	M3u8BaseData m3u8_data_;
	std::string m3u8_content_;

	std::string uri_;
	M3U8Data data_;
	M3U8State parser_state_;
};

