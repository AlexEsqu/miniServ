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

enum e_requestState {
	PARSING_REQUEST_LINE,
	PARSING_HEADERS,
	PARSING_BODY,
	PARSING_BODY_CHUNKED,
	PARSING_DONE,
	FILLING_ONGOING,
	FILLING_DONE,
	SENDING_ONGOING,
	SENDING_DONE
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

	std::string					_unparsedHeaderBuffer;

	std::string					_method;			// could be set as the enum already ?
	// e_methods					_methodCode;
	std::string					_protocol;			// we only support HTTP/1.1
	std::string					_requestedFileName;	// for example "/home.html"
	std::map
		<std::string,
		std::string>			_requestHeaderMap;

	const ServerConf&			_conf;
	const Route*				_route;
	size_t						_contentLength;
	Status						_status;

	e_requestState				_requestState;

	Buffer 						_requestBodyBuffer;

	Response*					_response;

	//-------------- INTERNAL FUNCTIONS -------------------//



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
	void				addAsHeaderVar(std::string& keyValueString);
	void				setIfParsingBody();
	void				setRoute(const Route* route);
	void				setResponse(Response* response);
	void				setParsingState(e_requestState requestState);

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
	Response*			getResponse();

	bool				isKeepAlive();

	//------------------- OPERATORS ----------------------//

	Request&			operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	void				addRequestChunk(std::string chunk);
	e_dataProgress		parseRequestLine(std::string& chunk);
	e_dataProgress		parseHeaderLine(std::string& chunk);
	e_dataProgress		parseRequestBody(std::string& chunk);
	e_dataProgress		parseChunkedBody(std::string& chunk);
	e_dataProgress		parseChunkedBody(std::istream& in);

	const Route*		findMatchingRoute();

};


