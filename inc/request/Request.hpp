#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>

#include "readability.hpp"
#include "ServerConf.hpp"

static const std::string httpRequestHeaderEnding("\r\n\r\n");

class Request;

class Request
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::string			_httpHeader;
	std::string			_httpBody;
	std::string			_fullRequest;		// full content of the request
	std::string			_method;			// could be set as the enum already ?
	std::string			_protocol;			// we only support HTTP/1.1
	std::string			_requestedFileName;	// for example "/home.html"
	std::map
		<std::string,
		std::string>	_additionalHeaderInfo;

	const ServerConf&	_conf;
	size_t				_byteRead;
	size_t				_contentLength;		// set at 0 if absent from Request
	int					_status;

	bool				_isHeaderComplete;

	//-------------- INTERNAL FUNCTIONS -------------------//

	void				checkHTTPValidity();

public:

	//----------------- CONSTRUCTORS ---------------------//

	Request(const ServerConf& conf, std::string httpRequest, size_t byteRead);
	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//-------------------- SETTER ------------------------//

	void				setMethod(std::string& httpRequest);
	void				setProtocol(std::string& httpRequest);
	void				setURI(std::string& httpRequest);
	void				setRequestLine(std::string& httpRequest);
	void				addAdditionalHeaderInfo(std::string& keyValueString);
	void				setContentLength();

	//-------------------- GETTERS -----------------------//

	std::string			getMethod() const;
	std::string			getProtocol() const;
	std::string			getRequestedURL() const;
	std::map<std::string, std::string>&	getAdditionalHeaderInfo();
	const ServerConf&	getConf() const;
	int					getStatus() const;

	bool				isKeepAlive();
	bool				isComplete();
	bool				isHeaderComplete();

	//------------------- OPERATORS ----------------------//

	Request&			operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	void				decodeRequestHeader(std::string &httpRequest);
	void				addRequestChunk(std::string httpRequest, size_t byteRead);

};


