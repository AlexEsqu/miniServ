#include "Request.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Request::Request(std::string httpRequest)
	: _fullRequest(httpRequest)
{
	decodeHTTPRequest(httpRequest);

	#ifdef DEBUG
		std::cout << "Request Constructor called" << std::endl;
		std::cout << "Method is [" << _method << "]\n";
		std::cout << "URL is [" << _requestedURL << "]\n";
		std::cout << "Protocol is [" << _protocol << "]\n";
		std::cout << "Content type is [" << _contentType << "]\n";
	#endif

}

Request::Request(const Request &copy)
{
	#ifdef DEBUG
		std::cout << "Request copy Constructor called" << std::endl;
	#endif
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

Request::~Request()
{
	#ifdef DEBUG
		std::cout << "Request Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

Request &Request::operator=(const Request &other)
{
	if (this == &other)
		return *this;
	_method = other._method;
	return *this;
}

//---------------------------- GUETTERS -------------------------------------//


//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//

void		Request::decodeHTTPRequest(std::string &httpRequest)
{
	std::string::iterator curr = httpRequest.begin();

	// Extract mandatory info
	extractMethodFromHTTP(curr);
	extractURLFromHTTP(curr);
	extractProtocolFromHTTP(curr);

	// check if Request is valid
	// TO DO

	// Extract additional info
	std::string	contentType = "content-type:";

	_contentType = extractInfoFromHTTPHeader(httpRequest, contentType);
}

// assumes the HTTP method is the first characters of the request until a space occurs
std::string	Request::extractMethodFromHTTP(std::string::iterator &it)
{
	std::string	httpMethod = "";
	while (*it != ' ') {
		httpMethod.append(1, *it);
		it++;
	}
	it++;
	_method = httpMethod;
	return (httpMethod);
}

std::string	Request::extractURLFromHTTP(std::string::iterator &it)
{
	std::string	fullURL = "";

	while (*it != ' ') {
		fullURL.append(1, *it);
		it++;
	}
	it++;
	_requestedURL = fullURL;
	return (fullURL);
}

std::string	Request::extractProtocolFromHTTP(std::string::iterator &it)
{
	std::string	protocol = "";

	// end of protocol is expected to be '\r' or carraige return
	while (*it != ' ' && std::isprint(*it)) {
		protocol.append(1, *it);
		it++;
	}
	it++;
	_protocol = protocol;
	return (protocol);
}

std::string	Request::extractInfoFromHTTPHeader(std::string &htmlRequest, std::string &infoType)
{
	std::string	result = "";

	size_t index = htmlRequest.find(infoType, 0);
	if (index == std::string::npos)
		return ("");

	std::string::iterator it = htmlRequest.begin() + index + infoType.size();
	// skip if space between infotype and info
	if (*it == ' ')
		it++;
	// store info into result until newline
	while (std::isprint(*it)) {
		result.append(1, *it);
		it++;
	}

	return (result);
}
