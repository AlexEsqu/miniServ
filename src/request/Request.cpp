#include "Request.hpp"
#include "readability.hpp"
#include "server.hpp"
#include "parsing.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

// Request::Request()
// 	: _fullRequest(""), _conf()
// {
// #ifdef DEBUG
// 	std::cout << "Request Generic Constructor called" << std::endl;
// #endif
// }

Request::Request(const ServerConf& conf,std::string httpRequest)
	: _fullRequest(httpRequest)
	, _conf(conf)
{
#ifdef DEBUG
	std::cout << "Request Constructor called" << std::endl;
#endif
	decodeHTTPRequest(httpRequest);
}

Request::Request(const Request &copy)
: _conf(copy._conf)
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
	if (this != &other) {
		_fullRequest = other._fullRequest;
		_method = other._method;
		_protocol = other._protocol;
		_requestedFileName = other._requestedFileName;
		_additionalHeaderInfo = other._additionalHeaderInfo;
	}

	return *this;
}

//---------------------------- GUETTERS -------------------------------------//

std::string		Request::getMethod() const
{
	return _method;
}

std::string		Request::getProtocol() const
{
	return _protocol;
}

std::string		Request::getRequestedURL() const
{
	return _requestedFileName;
}

std::map<std::string, std::string>&	Request::getAdditionalHeaderInfo()
{
	return _additionalHeaderInfo;
}

const ServerConf&	Request::getConf() const
{
	return _conf;
}

//----------------------- SETTERS -----------------------------------//

void	Request::setMethod(std::string &method)
{
	_method = method;
}

void	Request::setURI(std::string &URI)
{
	_requestedFileName = URI;
}

void	Request::setProtocol(std::string &protocol)
{
	_protocol = protocol;
}

// Valid request line (1st line of a HTTP request) must have the format:
//    Method SP Request-URI SP HTTP-Version CRLF
void	Request::setRequestLine(std::string &requestLine)
{
	std::vector<std::string> splitRequestLine = split(requestLine, ' ');
	if (splitRequestLine.size() != 3)
		throw badSyntax();
	setMethod(trim(splitRequestLine[0]));
	setURI(trim(splitRequestLine[1]));
	setProtocol(trim(splitRequestLine[2]));
}

void	Request::addAdditionalHeaderInfo(std::string &keyValueString)
{
	size_t	equalPos = keyValueString.find(':');

	if (equalPos != std::string::npos) {
		std::string key = keyValueString.substr(0, equalPos);
		std::string value = keyValueString.substr(equalPos + 1);
		key = trim(key);
		value = trim(value);
		_additionalHeaderInfo[key] = value;
		#ifdef DEBUG
			std::cout << "adding env var as [" << key << " = " << value << "]\n";
		#endif
	}
}

//----------------------- INTERNAL FUNCTIONS -----------------------------------//

void	Request::checkHTTPValidity()
{
	#ifdef DEBUG
		std::cout << "Checking validity:" << std::endl;
		std::cout << "Method is [" << _method << "]\n";
		std::cout << "URL is [" << _requestedFileName << "]\n";
		std::cout << "Protocol is [" << _protocol << "]\n";
	#endif

	// CHECK METHOD
	// empty method is not valid HTTP request
	if (getMethod().empty())
		throw badSyntax();
	// TO DO : check if within allowed method for the route, requires config class

	// CHECK PROTOCOL
	// throwing error if protocol is any other protocol than HTTP/1.1
	if (getProtocol() != "HTTP/1.1")
		throw badProtocol();

	// CHECK URL
	// empty URL is not valid HTTP request
	if (getRequestedURL().empty())
		throw badSyntax();

	// CHECK FORMAT
	// // check for the end of request Carriage Return or '\r'
	// if (httpRequest[httpRequest.size() - 2] != '\r')
	// 	throw badSyntax();
	// // check for the Line Feed or '\n'
	// if (httpRequest[httpRequest.size() - 1] != '\n')
	// 	throw badSyntax();
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//

// Goes through the Request Header line by line
// to set method, uri, protocol and env
void Request::decodeHTTPRequest(std::string &httpRequest)
{
	std::stringstream httpRequestStream(httpRequest);
	std::string line;
	bool isFirstLine = true;

	while (getline(httpRequestStream, line)) {

		// Removes trailing '\r' if present
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		// If there is an empty line, it is the end of the http headers
		if (line.empty()) {
			break;
		}

		// setting values in the request line for first line, then adding to env
		if (isFirstLine) {
			setRequestLine(line);
			isFirstLine = false;
		} else {
			addAdditionalHeaderInfo(line);
		}
	}

	checkHTTPValidity();

	#ifdef DEBUG
		std::cout << "Request HTTP was [" << httpRequest << "]\n";
		std::cout << "Method is [" << _method << "]\n";
		std::cout << "URL is [" << _requestedFileName << "]\n";
		std::cout << "Protocol is [" << _protocol << "]\n";
	#endif
}

//------------------------ EXCEPTIONS ---------------------------------//




