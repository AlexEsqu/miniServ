#include "Request.hpp"
#include "readability.hpp"
#include "server.hpp"

std::vector<std::string> split (const std::string &s, char delim) {
	std::vector<std::string> result;
	std::stringstream ss (s);
	std::string item;

	while (getline (ss, item, delim)) {
		result.push_back (item);
	}

	return result;
}

//--------------------------- CONSTRUCTORS ----------------------------------//

Request::Request()
	: _fullRequest("")
{
}

Request::Request(std::string httpRequest)
	: _fullRequest(httpRequest)
{
	decodeHTTPRequest(httpRequest);
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

	_fullRequest = other._fullRequest;
	decodeHTTPRequest(_fullRequest);

	return *this;
}

//---------------------------- GUETTERS -------------------------------------//

std::string Request::getMethod() const
{
	return _method;
}

std::string Request::getProtocol() const
{
	return _protocol;
}

std::string Request::getHost() const
{
	return _host;
}

std::string Request::getConnection() const
{
	return _connection;
}

std::string Request::getRequestedURL() const
{
	return _requestedFileName;
}

std::string Request::getContentType() const
{
	return _contentType;
}

Environment	Request::getRequestEnv()
{
	return _requestEnv.getPHPEnv();
}

int Request::getCGI() const
{
	return (_CGI);
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
void	Request::setRequestLine(std::string &httpRequest)
{
	std::string	requestLine = httpRequest.substr(0, httpRequest.find('\r'));
	if (requestLine.empty())
		throw badSyntax();
	std::vector<std::string> splitRequestLine = split (requestLine, ' ');
	if (splitRequestLine.size() != 3)
		throw badSyntax();
	setMethod(splitRequestLine[0]);
	setURI(splitRequestLine[1].insert(0, "."));
	setProtocol(splitRequestLine[2]);
}

void	Request::setRequestEnv(std::string &keyValueString)
{
	_requestEnv.setAdditionalEnv(keyValueString);
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

// Goes through the Request Header line by line to set method, uri, protocol and env
void Request::decodeHTTPRequest(std::string &httpRequest)
{
	// extracts and sets the request Method, URI and Protocol
	// which must be set in the first line of the request, aka the Request Line
	std::stringstream	httpRequestStream(httpRequest);
	std::string			requestLine;
	getline(httpRequestStream, requestLine);

	// if (!requestLine.empty() && requestLine.back() == '\r') {
	// 	requestLine.pop_back();
	// }

	setRequestLine(requestLine);

	// extracts and sets all remaining header info in the request environ
	std::string	headerLine;
	while (getline(httpRequestStream, headerLine)) {
		setRequestEnv(headerLine);
	}

	// check if Request is valid (Has Method, Protocol, URL, crlf)
	checkHTTPValidity();

	#ifdef DEBUG
		std::cout << "Request HTTP was [" << httpRequest << "]\n";
		std::cout << "Request line was [" << requestLine << "]\n";
		std::cout << "Method is [" << _method << "]\n";
		std::cout << "URL is [" << _requestedFileName << "]\n";
		std::cout << "Protocol is [" << _protocol << "]\n";
		std::cout << "Content type is [" << _contentType << "]\n";
	#endif
}


std::string	Request::getInfoFromHTTPHeader(std::string &httpRequest, std::string &infoType)
{
	std::string result = "";

	size_t index = httpRequest.find(infoType, 0);
	if (index == std::string::npos)
		return ("");

	std::string::iterator it = httpRequest.begin() + index + infoType.size();
	// skip if space between infotype and info
	if (*it == ' ')
		it++;
	// store info into result until newline
	while (std::isprint(*it))
	{
		result.append(1, *it);
		it++;
	}

	return (result);
}

void Request::setCGI()
{
	std::vector<std::string> acceptedCGIs;

	acceptedCGIs.push_back(".py");
	acceptedCGIs.push_back(".php");
	std::vector<std::string>::iterator it;

	for (it = acceptedCGIs.begin(); it != acceptedCGIs.end(); it++)
	{
		std::size_t pos = this->_requestedFileName.find(*it);
		if (pos != std::string::npos)
		{
			if (this->_requestedFileName.substr(pos) == ".py")
				this->_CGI = PY;
			if (this->_requestedFileName.substr(pos) == ".php")
				this->_CGI = PHP;
		}
	}
	this->_CGI = NO_CGI;
}

void Request::redirectIfCGI() // OR SET CGI?
{
	std::vector<std::string> acceptedCGIs;

	acceptedCGIs.push_back(".py");
	acceptedCGIs.push_back(".php");
	std::vector<std::string>::iterator it;

	for (it = acceptedCGIs.begin(); it != acceptedCGIs.end(); it++)
	{
		std::size_t pos = this->_requestedFileName.find(*it);
		if (pos != std::string::npos)
		{
			if (this->_requestedFileName.substr(pos) == ".py" || this->_requestedFileName.substr(pos) == ".php")
				return (Request::handleCGI());
		}
	}
	std::cout << CGI_FORMAT("NO CGI REQUIRED") << std::endl;
}

void Request::handleCGI()
{
	std::cout << CGI_FORMAT("CGI REQUIRED") << std::endl;
}

void Request::testFilename()
{
	Status status;
	std::ifstream input(this->_requestedFileName.c_str()); // opening the file as the content for the response
	std::stringstream content;
	if (!input.is_open())
	{
		status.setStatusCode(404);
		std::cerr << RED << status << STOP_COLOR << std::endl;
	}
}

//------------------------ EXCEPTIONS ---------------------------------//

Request::HTTPError::HTTPError(Request& req, int status)
{
	Response res(req, status);

	_message = res.getHTTPResponse();
}

std::string Request::HTTPError::getErrorPage()
{
	return _message;
}

const char*		Request::timeout::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request Timeout");
}

const char*		Request::badSyntax::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request has bad syntax");
}

const char*		Request::missingLength::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request has no content length");
}

const char*		Request::contentTooLarge::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request content is too big");
}

const char*		Request::forbiddenMethod::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request has a forbidden method");
}

const char*		Request::badProtocol::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request uses unsupported protocol");
}


