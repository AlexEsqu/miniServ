#pragma once
#include <iostream>

class Response
{
private:
	int 			_statusNum;
	int 			_method;	  // potential enum for GET,POST,DELETE... / or could be string too??
	std::string 	_contentType; 
	unsigned int 	_contentLength;
	std::string		_content;
	std::string 	_url;
	std::string		_response;

public:
	Response();
	Response(const Response &copy);
	virtual ~Response();

	Response &operator=(const Response &other);

	void setStatusNum(int number);
	void setMethod(int method);
	void setContentType(std::string type);
	void setContentLength(int length);
	void setContent(std::string content);
	void setUrl(std::string url);
	void setResponse(std::string response);
};
