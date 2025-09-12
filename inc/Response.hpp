#pragma once
#include <iostream>
#include "server.hpp"

class Status; 
class Request;

class Response
{
private:
	int _statusNum;
	int _method; // potential enum for GET,POST,DELETE... / or could be string too??
	std::string _contentType;
	unsigned int _contentLength;
	std::string _content;
	std::string _requestedFileName;
	std::string _response;
	std::string _protocol;
	std::string _HTTPResponse;


public:
	Response();
	Response(Request &req, int status);
	Response(const Response &copy);
	virtual ~Response();

	Response &operator=(const Response &other);

	void setHTTPResponse();
	void setStatusNum(int number);
	void setMethod(int method);
	void setContentType(std::string type);
	void setContentLength(int length);
	void setContent(std::string content);
	void setUrl(std::string url);
	void setResponse(std::string response);
	void setProtocol(std::string protocol);

	std::string getHTTPResponse() const ;
	std::string createErrorPageContent(const Status &num);

};