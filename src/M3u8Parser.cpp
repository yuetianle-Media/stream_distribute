#include "stdafx.h"
#include "m3u8parser.h"


M3u8Parser::M3u8Parser(const std::string &content)
	:m3u8_content_(content)
{
	_strings_to_lines(m3u8_content_);
	int line_no = 0;
	for (auto &item : content_lines_)
	{
		if (v_str_is_start_with(item, M3U8_EXT_X_BYTERANGE))
		{
			//_parse_byterange(item);
			s_state["expect_segment"] = true;
		}
		else if (v_str_is_start_with(item, M3U8_EXT_X_TARGETDURAION))
		{
			RegexTextFinder finder;
			if (finder.find(item, "#(.+?):(.+$)"))
			{
				m3u8_data_.target_duration = atoi(finder[2].c_str());
			}
		}
		else if (v_str_is_start_with(item, M3U8_EXT_X_MEDIDA_SEQUENCE))
		{
			_parse_simple_parameter(item, "string");
		}
		//else if (v_str_is_start_with(item, M3U8_EXT_X_PROGRAM_DATE_TIME))
		//{
		//	//_parse_and_set_simple_parameter_raw_value(content, "date");
		//	//DATA_ITER iter = s_data.find("program_date_time");
		//	//if (iter == s_data.end())
		//	//{
		//		//插入时间的指针到map中
		//	//}
		//	//s_state.insert() //插入时间的指针到map中
		//}
		//else if (v_str_is_start_with(content, M3U8_EXT_X_DISCONTINUITY))
		//{
		//	s_state["discontinuity"] = true;
		//}
		//else if (v_str_is_start_with(content, M3U8_EXT_X_CUE_OUT))
		//{
		//	_parse_cueout(content);
		//	s_state["cue_out"] = true;
		//	s_state["cue_start"] = true;
		//}
		else if (v_str_is_start_with(item, M3U8_EXT_X_VERSION))
		{
			_parse_simple_parameter(item, "string");
		}
		//else if (v_str_is_start_with(item, M3U8_EXT_X_ALLOW_CACHE))
		//{
		//	_parse_simple_parameter(item, "string");
		//}
		//else if (v_str_is_start_with(item, M3U8_EXT_X_KEY))
		//{
		//	s_state["current_key"] = _parse_key(item);
		//	//s_data["key"] = s_data.get("key", s_state["current_key"]);
		//}
		else if (v_str_is_start_with(item, M3U8_EXTINF))
		{
			_parse_extintf(item, line_no, true);
			s_state["expect_segment"] = true;
		}
		//else if (v_str_is_start_with(item, M3U8_EXT_X_STREAM_INF))
		//{
		//	s_state["expect_playlist"] = true;
		//	_parse_stream_inf(item);
		//}
		//else if (v_str_is_start_with(item, M3U8_EXT_X_I_FRAME_STREAM_INF))
		//{
		//	_parse_i_frame_stream_inf(item);
		//}
		//else if (v_str_is_start_with(item, M3U8_EXT_X_MEDIA))
		//{
		//	_parse_media(item);
		//}
		//else if (v_str_is_start_with(item, M3U8_EXT_X_PLAYLIST_TYPE))
		//{
		//	_parse_simple_parameter(item, "string");
		//}
		//else if (v_str_is_start_with(item, M3U8_EXT_I_FRAME_ONLY))
		//{
		//	bool is_i_frame = true;
		//	s_data["is_i_frame_only"] = &is_i_frame;
		//}
		//else if (v_str_is_start_with(item, M3U8_EXT_IS_INDEPENDENT_SEGMENTS))
		//{
		//	bool is_independ = true;
		//	s_data["is_independent"] = &is_independ;
		//}
		else if (v_str_is_start_with(item, M3U8_EXT_X_ENDLIST))
		{
			s_state["is_endlist"] = true;
		}
		else if (item == "")
		{
			continue;
		}
		else if (s_state["expect_segment"])
		{
			_parse_ts_chunk(item);
			s_state["expect_segment"] = false;
		}
		//else if (s_state["expect_playlist"])
		//{
		//	_parse_variant_playlist(item);
		//	s_state["expect_playlist"] = false;
		//}
		line_no++;
	}
}

M3u8Parser::~M3u8Parser()
{

}

//std::map<std::string, ContentType> M3u8Parser::s_data;

std::map<std::string, bool> M3u8Parser::s_state;

//std::vector<std::string> M3u8Parser::get_ts_file_list()
//{
//	return m3u8_data_.segments;
//}

bool M3u8Parser::get_ts_file_list(std::vector<std::string> &ts_file_list)
{
	ts_file_list = move(m3u8_data_.segments);
	if (!ts_file_list.empty())
	{
		return true;
	}
	else
	{
		return false;
	}
}

float M3u8Parser::get_max_duration()
{
	return m3u8_data_.target_duration;
}

//int M3u8Parser::_parse_key(const std::string &content)
//{
//	return 0;
//}

int M3u8Parser::_parse_extintf(const std::string &content, const int &line_no, bool strict)
{
	std::string tmp = content;
	std::string dest = v_str_replace(tmp, M3U8_EXTINF + ":", "");
	std::vector<std::string> dest_list;
	v_str_split(dest, dest_list);
	int length = 0;
	for (auto item = dest_list.begin(); item != dest_list.end(); item++)
	{
		length++;
	}
	if (length < 2)
	{
		float duration = atof(dest_list.at(0).c_str());
	}
	return 0;
}

int M3u8Parser::_parse_ts_chunk(const std::string &content)
{
	m3u8_data_.segments.push_back(content);
	//s_data["ts_stream"].list_data.push_back(content);
	return 0;
}

//int M3u8Parser::_parse_attribute_list(const std::string prefix, const std::string &content, void* func)
//{
//
//	return 0;
//}
//
//int M3u8Parser::_parse_stream_inf(const std::string &content)
//{
//	return 0;
//
//}
//
//int M3u8Parser::_parse_i_frame_stream_inf(const std::string &content)
//{
//	return 0;
//
//}
//
//int M3u8Parser::_parse_media(const std::string &content)
//{
//	return 0;
//
//}
//
//int M3u8Parser::_parse_variant_playlist(const std::string &content)
//{
//	return 0;
//
//}
//
//int M3u8Parser::_parse_byterange(const std::string &content)
//{
//	return 0;
//}

int M3u8Parser::_parse_simple_parameter_raw_value(const std::string &content, const std::string &cast_to, bool normalize/*=false*/)
{
	std::vector<std::string> out_param;
	v_str_split(content, out_param, ":");
	int index = 0;
	std::string tag;
	std::string tag_content;
	for (auto &item : out_param)
	{
		if (index % 2)
		{
			tag_content = item;
			//if (cast_to == "string")
			//	//s_data[tag].normal_data = tag_content;
			//else if (cast_to == "list")
			//	//s_data[tag].list_data.push_back(tag_content);
		}
		else
		{
			tag = item;
		}
	}

	return 0;
}

int M3u8Parser::_parse_and_set_simple_parameter_raw_value(const std::string &content, const std::string &cast_to, bool normalize/*=false*/)
{
	std::vector<std::string> out_param;
	v_str_split(content, out_param, ":");
	int index = 0;
	std::string tag;
	std::string tag_content;
	for (auto &item : out_param)
	{
		if (index % 2)
		{
			tag_content = item;
			//if (cast_to == "string")
			//	s_data[tag].normal_data = tag_content;
			//else if (cast_to == "list")
			//	s_data[tag].list_data.push_back(tag_content);
		}
		else
		{
			tag = item;
		}
	}
	return 0;
}

int M3u8Parser::_parse_simple_parameter(const std::string &content, const std::string &cast_to)
{
	_parse_and_set_simple_parameter_raw_value(content, cast_to, true);
	return 0;
}

//int M3u8Parser::_parse_cueout(const std::string &content)
//{
//	return 0;
//}

int M3u8Parser::_strings_to_lines(const std::string &content)
{
	std::string tmp = content;
	std::string dest = v_str_replace(tmp, "\r\n", "\n");
	v_str_split(dest, content_lines_, "\n");
	return 0;
}
