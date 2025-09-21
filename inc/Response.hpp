#pragma once
#include <iostream>


#include "server.hpp"
#include "Request.hpp"

class Status;

class Response
{
private:

	//------------------ ATTRIBUTES ----------------------//

	int				_statusNum;
	int				_method;			// enum for GET,POST,DELETE...
	std::string		_contentType;
	unsigned int	_contentLength;
	std::string		_content;
	std::string		_response;
	std::string		_requestedFileName;
	std::string		_HTTPResponse;
	int				_CGI;
	Request			_request;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Response();
	Response(Request& req);
	Response(Request& req, int status);
	Response(const Response& copy);

	//----------------- DESTRUCTOR -----------------------//

	virtual	~Response();

	//-------------------- SETTER ------------------------//

	void			setHTTPResponse();
	void			setStatusNum(int number);
	void			setMethod(int method);
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
