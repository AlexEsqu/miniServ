#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>

#include "readability.hpp"
#include "ServerConf.hpp"
#include "Status.hpp"

enum e_parseState {
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
	UNSUPPORTED
};

enum e_dataProgress {
	WAITING_FOR_MORE,
	RECEIVED_ALL
};


class Request;

class ServerConf;

class Request
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::string					_httpBody;
	std::string					_unparsedBuffer;	// unparsed leftover from previous recv
	std::string					_method;			// could be set as the enum already ?
	std::string					_protocol;			// we only support HTTP/1.1
	std::string					_requestedFileName;	// for example "/home.html"
	std::map
		<std::string,
		std::string>			_requestHeaderMap;

	const ServerConf&			_conf;
	Route						_matchingRoute;

	size_t						_contentLength;
	Status						_status;


	e_parseState			_parsingState;


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
	const Status&		getStatus() const;
	int					getParsingState() const;

	bool				isKeepAlive();

	//------------------- OPERATORS ----------------------//

	Request&			operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	void				addRequestChunk(std::string chunk);
	e_dataProgress		parseRequestLine();
	e_dataProgress		parseHeaderLine();
	e_dataProgress		parseRequestBody();
	e_dataProgress		parseChunkedBody();

};


