#include "Request.hpp"
#include "readability.hpp"
#include "server.hpp"
#include "parsing.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Request::Request(const ServerConf& conf, std::string httpRequest, size_t byteRead)
	: _fullRequest(httpRequest)
	, _conf(conf)
	, _byteRead(byteRead)
	, _contentLength(0)
	, _isHeaderComplete(false)
{
	if (isHeaderComplete())
	{
		extractHeader(httpRequest);
		decodeRequestHeader();
	}

}

Request::Request(const Request &copy)
: _conf(copy._conf)
{
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

Request::~Request()
{
}

//---------------------------- OPERATORS ------------------------------------//

Request &Request::operator=(const Request &other)
{
	if (this != &other) {
		_fullRequest = other._fullRequest;
		_method = other._method;
		_protocol = other._protocol;
		_requestedFileName = other._requestedFileName;
		_requestHeaderMap = other._requestHeaderMap;
		_isHeaderComplete = other._isHeaderComplete;
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
	return _requestHeaderMap;
}

const ServerConf&	Request::getConf() const
{
	return _conf;
}

int					Request::getStatus() const
{
	return _status;
}

bool				Request::isKeepAlive()
{
	if (_requestHeaderMap.find("Connection") != _requestHeaderMap.end())
		return (_requestHeaderMap["Connection"] == "keep-alive");
	return false;
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

void	Request::setContentLength()
{
	if (_requestHeaderMap.find("Content-Length") != _requestHeaderMap.end())
		_contentLength = atoi(_requestHeaderMap["Content-Length"].c_str());
	else
		_contentLength = 0;
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

void	Request::addAsHeaderVar(std::string &keyValueString)
{
	size_t	equalPos = keyValueString.find(':');

	if (equalPos != std::string::npos) {
		std::string key = keyValueString.substr(0, equalPos);
		std::string value = keyValueString.substr(equalPos + 1);
		key = trim(key);
		value = trim(value);
		_requestHeaderMap[key] = value;
	}
}

//----------------------- INTERNAL FUNCTIONS -----------------------------------//

void	Request::checkHTTPValidity()
{
	// empty method is not valid HTTP request
	if (getMethod().empty())
		throw badSyntax();
	// TO DO : check if within allowed method for the route, requires config class

	// throwing error if protocol is any other protocol than HTTP/1.1
	if (getProtocol() != "HTTP/1.1")
		throw badProtocol();

	// empty URL is not valid HTTP request
	if (getRequestedURL().empty())
		throw badSyntax();
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//

void	Request::extractHeader(std::string &httpRequest)
{
	std::stringstream	httpRequestStream(httpRequest);
	std::string			line;

	while (getline(httpRequestStream, line))
	{
		// Removes trailing '\r' if present
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		// If there is an empty line, it is the end of the http headers
		if (line.empty())
			break;

		// adding each line, without their \n, in a string vector
		_httpHeader.push_back(line);
	}
}

// Goes through the Request Header vector (line by line)
// to set method, uri, protocol and possible env & request variables
void	Request::decodeRequestHeader()
{
	// the first line of a HTTP request is the request line
	setRequestLine(_httpHeader[0]);

	for (size_t i = 0; i < _httpHeader.size(); i++)
		addAsHeaderVar(_httpHeader[i]);

	#ifdef DEBUG
		std::cout << "Request " << _method << " for " << _requestedFileName;
	#endif
}

void	Request::addRequestChunk(std::string httpRequest, size_t byteRead)
{
	_fullRequest.append(httpRequest);
	_byteRead += byteRead;

	#ifdef DEBUG
		std::cout << "Request added " << byteRead << " out of " << _contentLength << " to " << _byteRead;
	#endif
}

// void	Request::extractBody(std::string &buffer)
// {
// 	if (!isHeaderComplete()) {
// 		return;
// 	}

// 	// find the end of headers (double CRLF)
// 	size_t headerEndPos = _fullRequest.find(httpRequestHeaderEnding);

// 	// if no CRLF but request headers are complete, chunked request
// 	if (headerEndPos != std::string::npos)
// 	{

// 	}
// 	else
// 	{

// 	}

// 	// Calculate where body starts
// 	size_t bodyStartPos = headerEndPos + httpRequestHeaderEnding.length();

// 	// Extract body if it exists
// 	if (bodyStartPos < _fullRequest.length()) {
// 		_httpBody = _fullRequest.substr(bodyStartPos);

// 		#ifdef DEBUG
// 			std::cout << "Extracted body: " << _httpBody.length()
// 						<< " bytes (expected: " << _contentLength << ")" << std::endl;
// 		#endif
// 	}
// }

bool				Request::isComplete()
{
	if (!isHeaderComplete())
		return (false);

	if (_method == "GET" || _method == "HEAD" || _method == "DELETE") {
		return true;
	}

	if (_contentLength)
		return (_byteRead >= _contentLength);
	else
		return (true);
}

bool				Request::isHeaderComplete()
{
	if (_isHeaderComplete)
		return (true);

	if (_fullRequest.find(httpRequestHeaderEnding) != std::string::npos){
		_isHeaderComplete = true;
		return (true);
	}

	return (false);
}




