#include "stdafx.h"
#include "M3u8Parser.h"

M3u8Parser::M3u8Parser(const std::string uri, const std::string &content)
	:m3u8_content_(content), uri_(uri)
{
	if (!content.empty())
	{
		boost::filesystem::path uri_path(uri.c_str());
		_strings_to_lines(m3u8_content_);
		int line_no = 0;
		M3U8Segment m3u8_segment;
		StreamInfo stream_info;
		MediaInfo media_info;
		m3u8_segment.base_uri = uri_path.remove_filename().string();
		m3u8_segment.uri = uri;
		
		RegexTextFinder finder;
		for (auto &item : content_lines_)
		{
			if (v_str_is_start_with(item, M3U8_EXT_X_BYTERANGE))
			{
				_parse_byterange(&m3u8_segment, item);
				parser_state_.expect_segments = true;
			}
			else if (v_str_is_start_with(item, M3U8_EXT_X_TARGETDURAION))
			{
				if (finder.find(item, "#(.+?):(.+$)"))
				{
					m3u8_data_.target_duration = atoi(finder[2].c_str());
					data_.target_duration = atof(finder[2].c_str());
				}
			}
			else if (v_str_is_start_with(item, M3U8_EXT_X_MEDIDA_SEQUENCE))
			{
				if (finder.find(item, "#(.+?:(//d+$))"))
				{
					data_.media_sequence = atoi(finder[2].c_str());
				}
			}
			else if (v_str_is_start_with(item, M3U8_EXT_X_PROGRAM_DATE_TIME))
			{
				if (finder.find(item, "#(.+?):(.+$)"))
				{
					m3u8_segment.program_data_time = finder[2];
				}
			}
			else if (v_str_is_start_with(content, M3U8_EXT_X_DISCONTINUITY))
			{
				m3u8_segment.discontinuity = true;
			}
			//else if (v_str_is_start_with(content, M3U8_EXT_X_CUE_OUT))
			//{
			//	_parse_cueout(content);
			//	s_state["cue_out"] = true;
			//	s_state["cue_start"] = true;
			//}
			else if (v_str_is_start_with(item, M3U8_EXT_X_VERSION))
			{
				if (finder.find(item, "#(.+?):(//d+$)"))
				{
					data_.version = atoi(finder[2].c_str());
				}
			}
			else if (v_str_is_start_with(item, M3U8_EXT_X_ALLOW_CACHE))
			{
				if (finder.find(item, "#(.+?):(.+$)"))
				{
					std::string is_cached = finder[2];
					if (0 == is_cached.compare("YES") || 0 == is_cached.compare("yes"))
					{
						data_.allow_cache = true;
					}
					else
					{
						data_.allow_cache = false;
					}
				}
			}
			//else if (v_str_is_start_with(item, M3U8_EXT_X_KEY))
			//{
			//	s_state["current_key"] = _parse_key(item);
			//	//s_data["key"] = s_data.get("key", s_state["current_key"]);
			//}
			else if (v_str_is_start_with(item, M3U8_EXTINF))
			{
				_parse_extintf(&m3u8_segment, item, line_no, true);
				parser_state_.expect_segments = true;
			}
			else if (v_str_is_start_with(item, M3U8_EXT_X_STREAM_INF))
			{
				parser_state_.expect_playlist = true;
				_parse_stream_inf(&stream_info, item);
			}
			//else if (v_str_is_start_with(item, M3U8_EXT_X_I_FRAME_STREAM_INF))
			//{
			//	_parse_i_frame_stream_inf(item);
			//}
			else if (v_str_is_start_with(item, M3U8_EXT_X_MEDIA))
			{
				_parse_media(&media_info, item);
			}
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
				data_.is_end_list = true;
			}
			else if (item == "")
			{
				continue;
			}
			else if (v_str_is_start_with(item, "#"))//文件开始
			{
				continue;
			}
			else if (parser_state_.expect_segments)
			{
				_parse_ts_chunk(&m3u8_segment, item);
				parser_state_.expect_segments = false;
			}
			else if (parser_state_.expect_playlist)
			{
				_parse_variant_playlist(&stream_info, &media_info, item);
				parser_state_.expect_playlist = false;
			}
			line_no++;
		}

	}
}

M3u8Parser::~M3u8Parser()
{

}

bool M3u8Parser::get_play_list(PlayListList *&play_list)
{
	if (data_.is_variant)
	{
		play_list = &data_.play_lists;
		return true;
	}
	else
	{
		return false;
	}

}

//std::map<std::string, ContentType> M3u8Parser::s_data;

bool M3u8Parser::get_segments(M3U8SegmentList *&segment_list)
{
	if (!data_.is_variant)
	{
		segment_list = &data_.segments;
		return true;
	}
	return false;
}

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
	return data_.target_duration;
	//return m3u8_data_.target_duration;
}

//int M3u8Parser::_parse_key(const std::string &content)
//{
//	return 0;
//}

int M3u8Parser::_parse_extintf(M3U8Segment *segment, const std::string &content, const int &line_no, bool strict)
{
	if (!content.empty() && segment)
	{
		std::string tmp = content;
		std::string dest = v_str_replace(tmp, M3U8_EXTINF + ":", "");
		std::vector<std::string> dest_list;
		v_str_split(dest, dest_list);
		int length = dest_list.size();
		//for (auto item = dest_list.begin(); item != dest_list.end(); item++)
		//{
		//	length++;
		//}
		if (length == 1)
		{
			segment->duration = atof(dest_list.at(0).c_str());
		}
		else if (2 == length)
		{
			segment->title = dest_list.at(1);
		}
		return E_OK;
	}
	else
	{
		return E_PARAM_ERROR;
	}
}

int M3u8Parser::_parse_ts_chunk(M3U8Segment *segment, const std::string &content)
{
	m3u8_data_.segments.push_back(content);
	if (segment && !content.empty())
	{
		if (v_str_is_start_with(content, "HTTP") || v_str_is_start_with(content, "http"))
		{
			segment->uri = content;
		}
		else
		{
			std::string whole_path;
			whole_path.append(segment->base_uri).append("/").append(content);
			segment->uri = whole_path;
		}
		data_.segments.push_back(*segment);
		return E_OK;
	}
	else
	{
		return E_PARAM_ERROR;
	}
	//s_data["ts_stream"].list_data.push_back(content);
}

//int M3u8Parser::_parse_attribute_list(const std::string prefix, const std::string &content, void* func)
//{
//
//	return 0;
//}
//
int M3u8Parser::_parse_stream_inf(StreamInfo *stream_info, const std::string &content)
{
	data_.is_variant = true;
	data_.media_sequence = -1;
	std::string dest_str = v_str_remove_quotes(content);
	//替换字符串里面的逗号 example "A=\"aad,jld\""
	std::string stream_info_content = v_str_replace(dest_str, M3U8_EXT_X_STREAM_INF + ":", "");
	std::vector<std::string> value_items;
	v_str_split(stream_info_content, value_items, ",");
	RegexTextFinder finder;
	for (auto item : value_items)
	{
		if (finder.find(item, "(.+?)=(.+$)"))
		{
			std::string content_value = finder[1];
			if (std::string::npos != content_value.find(EXT_STREAM_INFO_PROGRAM_ID_NAME))
			{
				stream_info->program_id = atol(finder[2].c_str());
			}
			else if (std::string::npos != content_value.find(EXT_STREAM_INFO_BANDWIDTH_NAME))
			{
				stream_info->band_width = atol(finder[2].c_str());
			}
			else if (std::string::npos != content_value.find(EXT_STREAM_INFO_AVERAGE_BANDWIDTH_NAME))
			{
				stream_info->average_band_width = atol(finder[2].c_str());
			}
			else if (std::string::npos != content_value.find(EXT_STREAM_INFO_RESOLUTION_NAME))
			{
				stream_info->resolution = finder[2];
			}
			else if (std::string::npos != content_value.find(EXT_STREAM_INFO_CODEC_NAME))
			{
				stream_info->codec = finder[2];
			}
			else if (std::string::npos != content_value.find(EXT_STREAM_INFO_VIDEO_NAME))
			{
				stream_info->video = finder[2];
			}
			else if (std::string::npos != content_value.find(EXT_STREAM_INFO_AUDIO_NAME))
			{
				stream_info->audio = finder[2];
			}
			else if (std::string::npos != content_value.find(EXT_STREAM_INFO_SUBTITLES_NAME))
			{
				stream_info->subtitles = finder[2];
			}
		}
	}
	return 0;
}
//
//int M3u8Parser::_parse_i_frame_stream_inf(const std::string &content)
//{
//	return 0;
//
//}
//
int M3u8Parser::_parse_media(MediaInfo*media_info, const std::string &content)
{
	return 0;

}
//
int M3u8Parser::_parse_variant_playlist(StreamInfo*stream_info, MediaInfo*media_info, const std::string &content)
{
	PlayList stream_play;

	boost::filesystem::path uri_path(uri_.c_str());
	stream_play.base_uri = uri_path.remove_filename().string();

	if (!content.empty())
	{
		std::string whole_path(stream_play.base_uri);
		if (v_str_is_start_with(content, "HTTP")||v_str_is_start_with(content, "http"))
		{
			stream_play.uri = content;
			boost::filesystem::path uri_path(content.c_str());
			stream_play.base_uri = uri_path.remove_filename().string();
		}
		else
		{
			stream_play.uri = whole_path.append("/").append(content);
		}
		if (stream_info)
		{
			stream_play.stream_info = *stream_info;
		}
		if (media_info)
		{
			stream_play.media_info = *media_info;
		}
		data_.play_lists.push_back(stream_play);
		return E_OK;
	}
	else
	{
		return E_PARAM_ERROR;
	}
}

int M3u8Parser::_parse_byterange(M3U8Segment *segment, const std::string &content)
{
	if (!content.empty() && segment)
	{
		RegexTextFinder finder;
		std::string byte_range_pattern = M3U8_EXT_X_BYTERANGE + ":" + "(\\d+)@(\\d+)";
		if (finder.find(content, byte_range_pattern))
		{
			long int video_size = atoi(finder[1].c_str());
			long int video_offset = atoi(finder[2].c_str());
			segment->video_data_offset = video_offset;
			segment->video_size = video_size;
			return E_OK;
		}
		return E_PARAM_ERROR;
	}
	else
	{
		return E_PARAM_ERROR;
	}
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
