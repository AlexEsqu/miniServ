#pragma once
#include <iostream>
#include <fstream>

#include "Request.hpp"
#include "Status.hpp"
#include "Route.hpp"
#include "Buffer.hpp"
#include "Router.hpp"
#include "ServerSocket.hpp"
class Request;

class Response
{
private:
	//------------------ ATTRIBUTES ----------------------//

	Request*			_request;
	Status&				_status;

	std::string			_routedPath;
	std::map
		<std::string,
		std::string>	_mapOfHeadersToBeAdded;

	std::string			_contentType;
	size_t				_contentLength;
	std::string			_boundary;			//possible delimiter if POST form
	std::string			_HTTPHeaders;

	Buffer				_responsePage;
	size_t				_byteSent;

	std::string		createErrorPageContent(const Status &num); // in private jail to forbid Alex from using it by mistake

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

	void			setStatus(e_status number);
	void			setError(e_status number);

	void			setHeader(std::string key, std::string value);
	void			setContent(std::string content);

	void			setRoutedUrl(std::string url);

	void			setRequest(Request* request);
	void			setContentType(std::string type);
	void			setContentLength(int length);

	//-------------------- GETTERS -----------------------//

	Request*		getRequest();
	Status&			getStatus();

	std::string		getHTTPHeaders() const;
	std::string		getRoutedURL() const;

	std::string		getHTTPResponse();
	std::string		getBoundary();

	bool			hasError() const;

	//--------------- MEMBER FUNCTIONS -------------------//

	std::string		fetchErrorPageContent(const Status &num);
	void			addToContent(std::string contentChunk);
	void			addHttpHeader(std::string key, std::string value);
};
