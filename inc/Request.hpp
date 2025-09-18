#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>

#include "readability.hpp"
#include "EnvironmentBuilder.hpp"

class Request;

class Request
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::string			_fullRequest;		// full content of the request
	std::string			_method;			// could be set as the enum already ?
	std::string			_protocol;			// we only support HTTP/1.1
	std::string			_requestedFileName;	// for example "/home.html"
	std::map<std::string, std::string>	_additionalHeaderInfo;

	//-------------- INTERNAL FUNCTIONS -------------------//

	void			checkHTTPValidity();

public:

	//----------------- CONSTRUCTORS ---------------------//

	Request(); // empty constructor for testing purposes
	Request(std::string httpRequest);
	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//-------------------- SETTER ------------------------//

	void			setMethod(std::string& httpRequest);
	void			setProtocol(std::string& httpRequest);
	void			setURI(std::string& httpRequest);
	void			setRequestLine(std::string& httpRequest);
	void			addAdditionalHeaderInfo(std::string& keyValueString);

	//-------------------- GETTERS -----------------------//

	std::string		getMethod() const;
	std::string		getProtocol() const;
	std::string		getRequestedURL() const;
	std::map<std::string, std::string>&	getAdditionalHeaderInfo();

	//------------------- OPERATORS ----------------------//

	Request&		operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	void			decodeHTTPRequest(std::string &httpRequest);

	//------------------ EXCEPTIONS ----------------------//

};


