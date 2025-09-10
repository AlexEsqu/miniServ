#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <server.hpp>
class Request
{

private:
	//------------------ ATTRIBUTES ----------------------//

	std::string _fullRequest;  // full content of the request
	std::string _method;	   // could be set as the enum already ?
	std::string _protocol;	   // probably not needed, unless we only support HTTP/1.1
	std::string _host;		   // for example "example.com"
	std::string _connection;   // to research keep-alive
	std::string _requestedURL; // for example "/home.html"
	std::string _contentType;
	int _CGI;					// 0 NO CGI, 1 PY, 2 PHP
public:
	//----------------- CONSTRUCTORS ---------------------//

	Request(std::string httpRequest);
	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//-------------------- GETTERS -----------------------//

	std::string getMethod() const;
	std::string getProtocol() const;
	std::string getHost() const;
	std::string getConnection() const;
	std::string getRequestedURL() const;
	std::string getContentType() const;
	int 		getCGI() const;
	//------------------- OPERATORS ----------------------//

	Request &operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	void decodeHTTPRequest(std::string &httpRequest);
	std::string extractMethodFromHTTP(std::string::iterator &it);
	std::string extractProtocolFromHTTP(std::string::iterator &it);
	std::string extractURLFromHTTP(std::string::iterator &it);
	std::string extractInfoFromHTTPHeader(std::string &htmlRequest, std::string &infoType);

	void 		setCGI();
	void redirectIfCGI();
	void handleCGI(){};
};
