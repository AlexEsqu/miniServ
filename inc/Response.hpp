#pragma once
#include <iostream>
#include "server.hpp"

class Status;

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

	std::string createErrorPageContent(const Status &num);

public:
	Response();
	Response(int status, std::string contentType, std::string requestedUrl);
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
};

/*
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 319

<!DOCTYPE html>
<html lang="en">
<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Document</title>
		<link href="./styles.css" rel="stylesheet" type="text/css" />
</head>
<body>
		<p>Hello</p>
		<img src="./big-yoshi-wallpaper.png" width="500">
</body>
</html>

*/
