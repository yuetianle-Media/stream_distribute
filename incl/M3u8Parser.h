#pragma once
#include "M3u8define.h"
#include <map>
#include <vector>
#include <iostream>
//warning not thread safe.

inline bool v_str_is_start_with(const std::string &src, const std::string &flag)
{
	int index = src.find_first_of(flag);
	if (std::string::npos != index && index == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
};

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
		std::cout << "replacexxx:" << pos_begin << " " << pos << std::endl;
#endif
		ret.append(src.data() + pos_begin, pos - pos_begin);
		ret += dest;
		pos_begin = pos + 1;
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
		, is_independent_segments(false)
	{

	}
}M3u8NormalData;
class M3u8Parser
{
public:
	M3u8Parser(const std::string &content);
	~M3u8Parser();
	//static M3u8NormalData s_data;
	static std::map<std::string, ContentType>s_data;
	typedef std::map<std::string, ContentType>::iterator DATA_ITER;
	static std::map<std::string, bool> s_state;
private:
	int _parse_key(const std::string &content);
	int _parse_extintf(const std::string &content, const int &line_no, bool strict);
	int _parse_ts_chunk(const std::string &content);
	int _parse_attribute_list(const std::string prefix, const std::string &content, void* func);
	int _parse_stream_inf(const std::string &content);
	int _parse_i_frame_stream_inf(const std::string &content);
	int _parse_media(const std::string &content);
	int _parse_variant_playlist(const std::string &content);
	int _parse_byterange(const std::string &content);
	int _parse_simple_parameter_raw_value(const std::string &content, const std::string &cast_to, bool normalize=false);
	int _parse_and_set_simple_parameter_raw_value(const std::string &content, const std::string &cast_to, bool normalize=false);
	int _parse_simple_parameter(const std::string &content, const std::string &cast_to);
	int _parse_cueout(const std::string &content);
	int _strings_to_lines(const std::string &content);

	std::vector<std::string> content_lines_;
	std::string m3u8_content_;
};

