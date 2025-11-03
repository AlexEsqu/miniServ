#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>
#include <set>
#include <utility>

#include "readability.hpp"
#include "ServerConf.hpp"
#include "Status.hpp"
#include "Buffer.hpp"
#include "Session.hpp"

// Used in Request object for machine state receiving and parsing of request chunks
enum e_requestState {
	EMPTY,
	PARSING_REQUEST_LINE,
	PARSING_HEADERS,
	PARSING_BODY,
	PARSING_DONE
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

class ServerSocket;

class Request
{
private:

	//------------------ ATTRIBUTES ----------------------//

	// CONFIGURATION APPLICABLE TO THE REQUEST

	ServerConf&			_conf;					// configuration of the server socket
	const Route*		_route;					// route matched through the URI

	// REQUEST CURRENT STATE

	e_requestState		_requestState;			// parsing state of the request (headers, body...)
	Status&				_status;				// keeps track of request status code

	// REQUEST DATA

	std::string			_methodAsString;		// type of request as string
	e_methods			_method;				// type of request parsed as enum
	std::string			_protocol;				// must be HTTP/1.1
	std::string			_URI;					// for example "/" or "/home.html"
	std::map
		<std::string,
		std::string>	_requestHeaderMap;		// key=value of all header variables
	bool				_isChunked;
	size_t				_contentLength;			// length of the request body to be expected
	std::string			_contentType;

	// REQUEST BUFFERS

	std::string			_unparsedBuffer;		// may store chunks of request header
	Buffer 				_requestBodyBuffer;		// stores the body of the request

	// CGI DATA

	std::string			_paramCGI;
	int					_readingEndOfCGIPipe;	// if CGI is needed, fd to read the result in
	time_t				_cgiStartTime;
	int					_cgiForkPid;



	// SESSION MANAGEMENT

	size_t				_sessionId;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Request(ServerConf& conf, Status& status);

	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//-------------------- SETTER ------------------------//

	void				reset();

	void				setMethod(std::string& method);
	void				setProtocol(std::string& protocol);
	void				setURI(std::string& uri);
	void				setRequestLine(std::string& requestLine);
	void				setContentLength(std::string &lengthAsStr);
	void				setKeepAlive(bool value);

	void				setRoute(const Route* route);

	void				setCgiStartTime();
	void				setCgiPipe(int pipeFd);
	void				setCgiForkPid(int forkPid);

	void				addAsHeaderVar(std::string& keyValueString);
	void				setContentType(std::string string);

	void				setIfAssemblingBody();
	void				setParsingState(e_requestState requestState);
	void				setError(e_status statusCode);
	void				setStatus(e_status statusCode);

	//-------------------- GETTERS -----------------------//

	const ServerConf&	getConf() const;
	Status&				getStatus();

	std::string			getMethodAsString() const;
	e_methods			getMethodCode() const;
	std::string			getProtocol() const;
	std::string			getRequestedURL() const;

	std::string			getCgiParam() const;
	int					getCgiForkPid() const;
	int					getCgiPipe() const;
	time_t				getCgiStartTime() const;

	const Route*		getRoute() const;
	std::map
		<std::string,
		std::string>&	getAdditionalHeaderInfo();
	std::string			getContentType() const;
	size_t				getContentLength() const;
	Buffer&				getBodyBuffer();

	std::map
		<size_t,
		Session>&		getSessionMap();

	int					getParsingState() const;
	bool				hasError() const;

	size_t				getSessionId() const;
	std::string			getStringSessionId() const;

	std::string			getBody() const;
	std::istream&		getStreamFromBodyBuffer();

	bool				hasSessionId() const;

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

	e_dataProgress		assembleBody(std::string& chunk);
	e_dataProgress		assembleChunkedBody(std::string& chunk);
	e_dataProgress		assembleUnChunkedBody(std::string& chunk);
};


