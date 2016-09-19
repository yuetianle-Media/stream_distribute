#pragma once
#include "pre_regex.h"
class URIParser
{
public:
	URIParser(const string &uri);
	~URIParser();
public:
	bool is_ready() const { return is_ready_; }
	std::string uri() const { return uri_; }
	std::string protocol() const { return protocol_; }
	std::string host() const { return host_; }
	int port() const { return port_; }
	std::string resource_path() const { return resource_; }
	std::string param_str() const { return params_str_; }
	std::string username() const { return user_; }
	std::string password() const { return password_; }
private:
	
	std::string uri_;
	std::string protocol_;
	std::string host_;
	std::string user_;
	std::string password_;
	int port_;
	std::string path_;
	std::string resource_;
	std::string params_str_;
	std::map<std::string, std::string> params;

	RegexTextFinder regex_finder_;

	bool is_ready_;
	bool _parser_user(const string &auth);
	/*
	 * host 未做合法性检查
	*/
	bool _parser_endpoint(const string &endpoint);
};

