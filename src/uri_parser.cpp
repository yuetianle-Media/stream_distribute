#include "stdafx.h"
#include "uri_parser.h"


URIParser::URIParser(const string &uri)
	:port_(-1), is_ready_(false), uri_(uri)
{
	if (!uri.empty())
	{
		if (regex_finder_.find(uri, "(.+?)://(.+?)/(.+)"))
		{
			protocol_ = regex_finder_[1];
			string endpoint = regex_finder_[2];
			string path = regex_finder_[3];
			if (string::npos != path.find("?"))
			{
				if (regex_finder_.find(path, "(.+?)\\?(.+$)"))
				{
					resource_ = regex_finder_[1];
					params_str_ = regex_finder_[2];
				}
			}
			else
			{
				resource_ = path;
			}
			if (regex_finder_.find(endpoint, "(.+?)@(.+$)"))
			{
				string auth = regex_finder_[1];
				_parser_user(auth);
				endpoint = regex_finder_[2];
			}
			if (_parser_endpoint(endpoint))
			{
				is_ready_ = true;
			}
		}
	}
}


URIParser::~URIParser()
{
}

bool URIParser::_parser_user(const string &auth)
{
	if (regex_finder_.find(auth, "(.+?):(.+$)"))
	{
		user_		= regex_finder_[1];
		password_	= regex_finder_[2];
		return true;
	}
	return false;
}

bool URIParser::_parser_endpoint(const string &endpoint)
{
	if (string::npos != endpoint.find(":"))
	{
		if (regex_finder_.find(endpoint, "(.+?):(\\d+)"))
		{
			host_ = regex_finder_[1];
			port_ = atoi(regex_finder_[2].c_str());
		}
		else
		{
			return false;
		}
	}
	else
	{
		host_ = endpoint;
		if (protocol_ == "http" || protocol_ == "HTTP")
		{
			port_ = 80;
		}
	}
	return true;
}
