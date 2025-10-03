#pragma once
#include <iostream>
#include <fstream>

#include "Request.hpp"
#include "Status.hpp"
#include "ServerConf.hpp"

class Request;

class Response
{
private:

	//------------------ ATTRIBUTES ----------------------//

	int					_statusNum;
	std::string			_method;			// enum for GET,POST,DELETE...
	std::string			_contentType;
	unsigned int		_contentLength;
	std::string			_content;
	std::string			_requestedFileName;
	std::string			_routedPath;
	std::string			_HTTPResponse;
	int					_CGI;
	Request*			_request;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Response();
	Response(Request* req);
	Response(Request* req, int status);
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
	void			setContent(std::vector<char> content);
	void			setUrl(std::string url);
	void			setResponse(std::string response);
	void			setAsRoute(Route& route);

	//-------------------- GETTERS -----------------------//

	std::string		getHTTPResponse() const ;
	Request*		getRequest();
	int				getStatus() const;

	//------------------- OPERATORS ----------------------//

	Response		&operator=(const Response& other);

	//--------------- MEMBER FUNCTIONS -------------------//

	std::string		createErrorPageContent(const Status& num);
	std::string		getRoutedURL() const;
	void			testFilename();
};
