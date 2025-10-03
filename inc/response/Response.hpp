#pragma once
#include <iostream>
#include <fstream>

#include "Request.hpp"
#include "Status.hpp"
#include "Route.hpp"

class Request;

class Response
{
private:

	//------------------ ATTRIBUTES ----------------------//

	int					_statusNum;
	Request*			_request;

	Route			_route;
	std::string			_routedPath;

	int					_CGI;

	std::string			_contentType;
	size_t				_contentLength;
	std::string			_content;
	std::string			_HTTPResponse;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Response();
	Response(Request* req);
	Response(Request* req, int status);
	Response(const Response& copy);

	//----------------- DESTRUCTOR -----------------------//

	virtual	~Response();

	//-------------------- SETTER ------------------------//

	void			AddHTTPHeaders();
	void			setStatusNum(int number);
	void			setContent(std::string content);
	void			setContent(std::vector<char> content);
	void			setRoutedUrl(std::string url);
	void			setResponse(std::string response);
	void			setRoute(const Route& route);
	void			setContentType(std::string type);
	void			setContentLength(int length);

	//-------------------- GETTERS -----------------------//

	std::string		getHTTPResponse() const ;
	Request*		getRequest();
	int				getStatus() const;
	const Route&	getRoute() const;

	//------------------- OPERATORS ----------------------//

	Response		&operator=(const Response& other);

	//--------------- MEMBER FUNCTIONS -------------------//

	std::string		createErrorPageContent(const Status& num);
	std::string		getRoutedURL() const;
	void			testFilename();
};
