#pragma once
#include <iostream>
#include <fstream>

#include "Request.hpp"
#include "Status.hpp"
#include "ServerConf.hpp"


enum e_methods
{
	GET,
	POST,
	DELETE,
	UNSUPPOTRTED
};

class Response
{
private:

	//------------------ ATTRIBUTES ----------------------//

	int				_statusNum;
	std::string			_method;			// enum for GET,POST,DELETE...
	std::string		_contentType;
	unsigned int	_contentLength;
	std::string		_content;
	std::string		_response;
	std::string		_requestedFileName;
	std::string		_HTTPResponse;
	int				_CGI;
	Request			_request;
	ServerConf&		_conf;

public:
	// Response();

	//----------------- CONSTRUCTORS ---------------------//

	Response(ServerConf& conf, Request& req);
	Response(ServerConf& conf, Request& req, int status);
	Response(const Response& copy);

	//----------------- DESTRUCTOR -----------------------//

	virtual	~Response();

	//-------------------- SETTER ------------------------//

	void			setHTTPResponse();
	void			setStatusNum(int number);
	void			setMethod(std::string method);
	void			setContentType(std::string type);
	void			setContentLength(int length);
	void			setContent(std::string content);
	void			setUrl(std::string url);
	void			setResponse(std::string response);

	//-------------------- GETTERS -----------------------//

	std::string		getHTTPResponse() const ;
	Request&		getRequest();
	std::string		getRoutedURL() const;

	//------------------- OPERATORS ----------------------//

	Response		&operator=(const Response& other);

	//--------------- MEMBER FUNCTIONS -------------------//

	std::string		createErrorPageContent(const Status& num);
	void			testFilename();
};
