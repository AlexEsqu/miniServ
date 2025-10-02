#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>

#include "readability.hpp"
#include "ServerConf.hpp"

enum RequestParseState {
	PARSING_REQUEST_LINE,
	PARSING_HEADERS,
	PARSING_BODY,
	PARSING_BODY_CHUNKED,
	PARSING_DONE
};

enum e_methods
{
	GET,
	POST,
	DELETE,
	UNSUPPOTRTED
};

enum e_parsProgress {
	WAITING_FOR_MORE,
	RECEIVED_ALL
};

class Request;

class Request
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::string					_httpBody;
	std::string					_unparsedBuffer;		// full content of the request
	std::string					_method;			// could be set as the enum already ?
	std::string					_protocol;			// we only support HTTP/1.1
	std::string					_requestedFileName;	// for example "/home.html"
	std::map
		<std::string,
		std::string>			_requestHeaderMap;

	const ServerConf&			_conf;
	size_t						_contentLength;
	int							_status;

	RequestParseState			_parsingState;

	//-------------- INTERNAL FUNCTIONS -------------------//

	void				checkHTTPValidity();

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

	//-------------------- GETTERS -----------------------//

	std::string			getMethod() const;
	std::string			getProtocol() const;
	std::string			getRequestedURL() const;
	std::map
		<std::string,
		std::string>&	getAdditionalHeaderInfo();

	const ServerConf&	getConf() const;
	int					getStatus() const;
	int					getParsingState() const;

	bool				isKeepAlive();

	//------------------- OPERATORS ----------------------//

	Request&			operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	void				addRequestChunk(std::string chunk);
	e_parsProgress		parseRequestLine();
	e_parsProgress		parseHeaderLine();
	e_parsProgress		parseRequestBody();
	e_parsProgress		parseChunkedBody();

};


