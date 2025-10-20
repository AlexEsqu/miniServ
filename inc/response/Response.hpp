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

	Request*			_request;
	Status&				_status;

	std::string			_routedPath;

	std::string			_contentType;
	size_t				_contentLength;
	std::string			_HTTPHeaders;

	Buffer				_responsePage;
	size_t				_byteSent;

public:
	//----------------- CONSTRUCTORS ---------------------//

	Response(Request *req);
	Response(const Response &copy);

	//------------------- OPERATORS ----------------------//

	Response		&operator=(const Response &other);

	//----------------- DESTRUCTOR -----------------------//

	virtual			~Response();

	//-------------------- SETTER ------------------------//

	void			createHTTPHeaders();

	void			setStatus(unsigned int number);
	void			setError(unsigned int number);

	void			setContent(std::string content);
	void			setRoutedUrl(std::string url);

	void			setRequest(Request* request);
	void			setContentType(std::string type);
	void			setContentLength(int length);

	//-------------------- GETTERS -----------------------//

	Request*		getRequest();
	int				getStatus() const;
	std::string		getHTTPHeaders() const;
	std::string		getRoutedURL() const;

	std::string		getHTTPResponse();

	//--------------- MEMBER FUNCTIONS -------------------//

	std::string		createErrorPageContent(const Status &num);
	std::string		fetchErrorPageContent(const Status &num);
	void			addToContent(std::string contentChunk);

	void			routeToDefaultFiles(std::string& url, const Route* route, std::string& root);
	void			routeToFilePath(std::string& url, const Route* route, std::string& root);
	void			routeUrlForGet(std::string url);
	void			routeUrlForPostDel(std::string url);
};
