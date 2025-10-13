#pragma once
#include <iostream>
#include <fstream>

#include "Request.hpp"
#include "Status.hpp"
#include "Route.hpp"
#include "Buffer.hpp"

class Request;

class Response
{
private:

	//------------------ ATTRIBUTES ----------------------//

	int					_statusNum;

	Request*			_request;

	std::string			_routedPath;

	int					_CGI;

	std::string			_contentType;
	size_t				_contentLength;
	std::string			_HTTPHeaders;

	Buffer				_responsePage;
	size_t				_byteSent;


public:

	//----------------- CONSTRUCTORS ---------------------//

	Response();
	Response(Request* req);
	Response(Request* req, int status);
	Response(const Response& copy);

	//----------------- DESTRUCTOR -----------------------//

	virtual	~Response();

	//-------------------- SETTER ------------------------//

	void			createHTTPHeaders();

	void			setStatusNum(int number);
	void			setContent(std::string content);
	void			setRoutedUrl(std::string url);

	// void			setResponse(std::string response);

	void			setRequest(Request* request);
	void			setContentType(std::string type);
	void			setContentLength(int length);

	//-------------------- GETTERS -----------------------//

	Request*		getRequest();
	int				getStatus() const;
	std::string		getHTTPHeaders() const;
	std::string		getRoutedURL() const;

	std::string		getHTTPResponseChunk(size_t size);

	//------------------- OPERATORS ----------------------//

	Response		&operator=(const Response& other);

	//--------------- MEMBER FUNCTIONS -------------------//

	std::string		createErrorPageContent(const Status& num);
	void			addToContent(std::string contentChunk);

};
