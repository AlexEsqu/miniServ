#include "Request.hpp"
#include "readability.hpp"
#include "server.hpp"
#include "parsing.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Request::Request(const ServerConf& conf, std::string requestChunk)
	: _fullRequest(requestChunk)
	, _conf(conf)
	, _parsingState(PARSING_REQUEST_LINE)
{
	addRequestChunk(requestChunk);
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


int					Request::getParsingState() const
{
	return _parsingState;
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

e_parsProgress	Request::parseRequestLine()
{
	size_t lineEnd = _fullRequest.find("\r\n");
	if (lineEnd == std::string::npos)
		return WAITING_FOR_MORE;
	std::string requestLine = _fullRequest.substr(0, lineEnd);
	setRequestLine(requestLine);
	_fullRequest.erase(0, lineEnd + 2);
	_parsingState = PARSING_HEADERS;
	return RECEIVED_ALL;
}

e_parsProgress	Request::parseHeaderLine()
{
	size_t lineEnd = _fullRequest.find("\r\n");
	if (lineEnd == std::string::npos)
		return WAITING_FOR_MORE;
	std::string headerLine = _fullRequest.substr(0, lineEnd);
	if (headerLine.empty())
	{
		setContentLength();
		if (shouldHaveBody())
			_parsingState = PARSING_BODY;
		else
			_parsingState = PARSING_DONE;
	}
	else
		addAsHeaderVar(headerLine);
	_fullRequest.erase(0, lineEnd + 2);
	return RECEIVED_ALL;
}

e_parsProgress	Request::parseRequestBody()
{
	if (_fullRequest.size() < _contentLength)
		return WAITING_FOR_MORE;
	_httpBody = _fullRequest.substr(0, _contentLength);
	_fullRequest.erase(0, _contentLength);
	_parsingState = PARSING_DONE;
	return RECEIVED_ALL;
}

void	Request::addRequestChunk(std::string chunk)
{
	_fullRequest.append(chunk);

	while (_parsingState != PARSING_DONE)
	{
		switch (_parsingState)
		{
			case PARSING_REQUEST_LINE:
			{
				if (parseRequestLine() == WAITING_FOR_MORE)
					return;
				break;
			}
			case PARSING_HEADERS:
			{
				if (parseHeaderLine() == WAITING_FOR_MORE)
					return;
				break;
			}
			case PARSING_BODY:
			{
				if (parseRequestBody() == WAITING_FOR_MORE)
					return;
				break;
			}
			case PARSING_DONE:
				return;
		}
	}
}

bool				Request::shouldHaveBody()
{
	if (_method == "GET" || _method == "HEAD" || _method == "DELETE") {
		return false;
	}
	return true;
}





