#pragma once
#include <iostream>
#include <string>

class Request
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::string	_fullRequest;	// full content of the request
	std::string	_method;		// could be set as the enum already ?
	std::string	_protocol;		// probably not needed, unless we only support HTTP/1.1
	std::string	_host;			// for example "example.com"
	std::string	_connection;	// to research keep-alive
	std::string	_requested_url;	// for example "/home.html"

public:

	//----------------- CONSTRUCTORS ---------------------//

	Request(std::string httpRequest);
	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//------------------- OPERATORS ----------------------//

	Request &operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	std::string	extractMethodFromHTTP(std::string httpRequest);
	std::string	extractProtocolFromHTTP(std::string httpRequest);

};
