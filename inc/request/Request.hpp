#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>
#include <set>

#include "readability.hpp"
#include "ServerConf.hpp"
#include "Status.hpp"
#include "Buffer.hpp"

// Used in Request object for machine state receiving and parsing of request chunks
enum e_requestState {
	PARSING_REQUEST_LINE,
	PARSING_HEADERS,
	PARSING_BODY,
	PARSING_DONE,
	FILLING_ONGOING,
	FILLING_DONE,
	SENDING_ONGOING,
	SENDING_DONE,
	HAS_ERROR
};

enum e_methods
{
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	UNSUPPORTED
};

// Used when receiving data from sockets, to indicate whether a chunk can be parsed
// or is missing informations and more content is needed
enum e_dataProgress {
	WAITING_FOR_MORE,
	RECEIVED_ALL
};

class Request;

class Response;

class ServerConf;

class Request
{
private:

	//------------------ ATTRIBUTES ----------------------//

	// REQUEST DATA

	std::string			_methodAsString;		// type of request as string
	e_methods			_method;				// type of request parsed as enum
	std::string			_protocol;				// must be HTTP/1.1
	std::string			_URI;					// for example "/" or "/home.html"

	std::map<std::string,std::string>	_requestHeaderMap; // key=value of all header variables
	bool				_isChunked;
	size_t				_contentLength;			// length of the request body to be expected
	std::string			_unparsedHeaderBuffer;	// may store chunks of request header
	Buffer 				_requestBodyBuffer;		// stores the body of the request

	// CONFIGURATION APPLICABLE TO THE REQUEST

	const ServerConf&	_conf;					// configuration of the server socket
	const Route*		_route;					// route matched through the URI
	std::string			_routedURI;				// for example "/var/www/html/home.html"
	std::string			_paramCGI;

	// REQUEST CURRENT STATE

	e_requestState		_requestState;			// current state of the request (parsing, fufilling, sending)
	Status				_status;				// keeps track of request status code

	// REQUEST RESULT

	Response*			_response;				// allocated when needed, stores the response content, generates headers

public:

	//----------------- CONSTRUCTORS ---------------------//

	Request(const ServerConf& conf, std::string requestChunk);
	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//-------------------- SETTER ------------------------//

	void				setMethod(std::string& method);
	void				setProtocol(std::string& protocol);
	void				setURI(std::string& uri);
	void				setRequestLine(std::string& requestLine);

	void				setRoute(const Route* route);

	void				addAsHeaderVar(std::string& keyValueString);

	void				setResponse(Response* response);

	void				setIfParsingBody();
	void				setParsingState(e_requestState requestState);
	void				setError(unsigned int statusCode);

	//-------------------- GETTERS -----------------------//

	std::string			getMethod() const;
	std::string			getProtocol() const;
	std::string			getRequestedURL() const;
	const Route*		getRoute() const;
	std::map
		<std::string,
		std::string>&	getAdditionalHeaderInfo();

	const ServerConf&	getConf() const;
	const Status&		getStatus() const;
	int					getParsingState() const;
	bool				hasError() const;
	Response*			getResponse();

	std::string			getBody() const;

	bool				isKeepAlive();

	//------------------- OPERATORS ----------------------//

	Request&			operator=(const Request &other);

	//----------------- VALIDATORS  ----------------------//

	void				validateRequestLine();
	void				checkMethodIsAllowed();

	//--------------- MEMBER FUNCTIONS -------------------//

	void				addRequestChunk(std::string chunk);
	e_dataProgress		parseRequestLine(std::string& chunk);
	e_dataProgress		parseHeaderLine(std::string& chunk);
	e_dataProgress		parseRequestBody(std::string& chunk);
	e_dataProgress		parseChunkedBody();

	const Route*		findMatchingRoute();
};


