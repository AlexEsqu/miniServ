#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

class Request
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::string					_fullRequest;	// full content of the request
	std::string					_method;		// could be set as the enum already ?
	std::string					_protocol;		// probably not needed, unless we only support HTTP/1.1
	std::string					_host;			// for example "example.com"
	std::string					_connection;	// to research keep-alive
	std::string					_requestedURL;	// for example "/home.html"
	std::string					_contentType;
	std::vector<std::string>	_requestEnv;	// all header variables as ENV in a vector

	//----------- PRIVATE MEMBER FUNCTION ----------------//

	std::string		extractMethodFromHTTP(std::string::iterator &it);
	std::string		extractProtocolFromHTTP(std::string::iterator &it);
	std::string		extractURLFromHTTP(std::string::iterator &it);
	std::string		getInfoFromHTTPHeader(std::string &httpRequest, std::string &infoType);
	void			fillEnvFromHTTPHeader(std::string &httpRequest, std::string::iterator &curr);
	bool			checkHTTPValidity(std::string &httpRequest, std::string::iterator &it);

public:

	//----------------- CONSTRUCTORS ---------------------//

	Request(std::string httpRequest);
	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//-------------------- GETTERS -----------------------//

	std::string		getMethod() const;
	std::string		getProtocol() const;
	std::string		getHost() const;
	std::string		getConnection() const;
	std::string		getRequestedURL() const;
	std::string		getContentType() const;
	char* const*	getRequestEnv() const;

	//------------------- OPERATORS ----------------------//

	Request &operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	void		decodeHTTPRequest(std::string &httpRequest);

};
