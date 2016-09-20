#include "stdafx.h"
#include "M3u8Parser.h"


M3u8Parser::M3u8Parser(const std::string &content)
	:m3u8_content_(content)
{
	_strings_to_lines(m3u8_content_);
	int line_no = 0;
	for (auto &item : content_lines_)
	{
		if (v_str_is_start_with(item, M3U8_EXT_X_BYTERANGE))
		{
			_parse_byterange(item);
			s_state["expect_segment"] = true;
		}
		else if (v_str_is_start_with(item, M3U8_EXT_X_TARGETDURAION))
		{
			_parse_simple_parameter(item, "float");
		}
		else if (v_str_is_start_with(item, M3U8_EXT_X_MEDIDA_SEQUENCE))
		{
			_parse_simple_parameter(item, "int");
		}
		else if (v_str_is_start_with(item, M3U8_EXT_X_PROGRAM_DATE_TIME))
		{
			_parse_and_set_simple_parameter_raw_value(content, "date");
			DATA_ITER iter = s_data.find("program_date_time");
			if (iter == s_data.end())
			{
				//插入时间的指针到map中
			}
			//s_state.insert() //插入时间的指针到map中
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_DISCONTINUITY))
		{
			s_state["discontinuity"] = true;
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_CUE_OUT))
		{
			_parse_cueout(content);
			s_state["cue_out"] = true;
			s_state["cue_start"] = true;
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_VERSION))
		{
			_parse_simple_parameter(content, "string");
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_ALLOW_CACHE))
		{
			_parse_simple_parameter(content, "string");
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_KEY))
		{
			s_state["current_key"] = _parse_key(content);
			//s_data["key"] = s_data.get("key", s_state["current_key"]);
		}
		else if (v_str_is_start_with(content, M3U8_EXTINF))
		{
			_parse_extintf(content, line_no, true);
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_STREAM_INF))
		{
			s_state["expect_playlist"] = true;
			_parse_stream_inf(content);
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_I_FRAME_STREAM_INF))
		{
			_parse_i_frame_stream_inf(content);
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_MEDIA))
		{
			_parse_media(content);
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_PLAYLIST_TYPE))
		{
			_parse_simple_parameter(content, "string");
		}
		else if (v_str_is_start_with(content, M3U8_EXT_I_FRAME_ONLY))
		{
			bool is_i_frame = true;
			s_data["is_i_frame_only"] = &is_i_frame;
		}
		else if (v_str_is_start_with(content, M3U8_EXT_IS_INDEPENDENT_SEGMENTS))
		{
			bool is_independ = true;
			s_data["is_independent"] = &is_independ;
		}
		else if (v_str_is_start_with(content, M3U8_EXT_X_ENDLIST))
		{
			bool is_end = true;
			s_data["is_endlist"] = &is_end;
		}
		else if (content == "")
		{
			continue;
		}
		else if (s_state["expect_segment"])
		{
			_parse_ts_chunk(content);
			s_state["expect_segment"] = false;
		}
		else if (s_state["expect_playlist"])
		{
			_parse_variant_playlist(content);
			s_state["expect_playlist"] = false;
		}
		line_no++;
	}
}

M3u8Parser::~M3u8Parser()
{
}

std::map<std::string, ContentType> M3u8Parser::s_data;

std::map<std::string, bool> M3u8Parser::s_state;

int M3u8Parser::_parse_key(const std::string &content)
{

}

int M3u8Parser::_parse_extintf(const std::string &content, const int &line_no, bool strict)
{

}

int M3u8Parser::_parse_ts_chunk(const std::string &content)
{

}

int M3u8Parser::_parse_attribute_list(const std::string prefix, const std::string &content, void* func)
{

}

int M3u8Parser::_parse_stream_inf(const std::string &content)
{

}

int M3u8Parser::_parse_i_frame_stream_inf(const std::string &content)
{

}

int M3u8Parser::_parse_media(const std::string &content)
{

}

int M3u8Parser::_parse_variant_playlist(const std::string &content)
{

}

int M3u8Parser::_parse_byterange(const std::string &content)
{

}

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
			if (cast_to == "string")
				s_data[tag].normal_data = tag_content;
			else if (cast_to == "list")
				s_data[tag].list_data.push_back(tag_content);
		}
		else
		{
			tag = item;
		}
	}

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
			if (cast_to == "string")
				s_data[tag].normal_data = tag_content;
			else if (cast_to == "list")
				s_data[tag].list_data.push_back(tag_content);
		}
		else
		{
			tag = item;
		}
	}
}

int M3u8Parser::_parse_simple_parameter(const std::string &content, const std::string &cast_to)
{
	_parse_and_set_simple_parameter_raw_value(content, cast_to, true);
}

int M3u8Parser::_parse_cueout(const std::string &content)
{

}

int M3u8Parser::_strings_to_lines(const std::string &content)
{
	std::string tmp = content;
	std::string dest = v_str_replace(tmp, "\r\n", "\n");
	v_str_split(dest, content_lines_, "\n");
	return 0;
}
