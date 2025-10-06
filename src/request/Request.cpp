#include "Request.hpp"
#include "readability.hpp"
#include "server.hpp"
#include "parsing.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Request::Request(const ServerConf& conf, std::string requestChunk)
	: _unparsedBuffer(requestChunk)
	, _conf(conf)
	, _route(NULL)
	, _status(200)
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
		_unparsedBuffer = other._unparsedBuffer;
		_method = other._method;
		_protocol = other._protocol;
		_requestedFileName = other._requestedFileName;
		_requestHeaderMap = other._requestHeaderMap;
		_route = other._route;
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

const Status&		Request::getStatus() const
{
	return _status;
}


int					Request::getParsingState() const
{
	return _parsingState;
}

const Route*		Request::getRoute() const
{
	return (_route);
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

void	Request::setRoute(const Route* route)
{
	_route = route;
}

// Valid request line (1st line of a HTTP request) must have the format:
//    Method SP Request-URI SP HTTP-Version CRLF
void	Request::setRequestLine(std::string &requestLine)
{
	std::vector<std::string> splitRequestLine = split(requestLine, ' ');
	if (splitRequestLine.size() != 3)
		throw HTTPError(this, 400);
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
	// // empty method is not valid HTTP request
	// if (getMethod().empty())
	// 	throw badSyntax();
	// // TO DO : check if within allowed method for the route, requires config class

	// // throwing error if protocol is any other protocol than HTTP/1.1
	// if (getProtocol() != "HTTP/1.1")
	// 	throw badProtocol();

	// // empty URL is not valid HTTP request
	// if (getRequestedURL().empty())
	// 	throw badSyntax();
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//

e_dataProgress	Request::parseRequestLine()
{
	size_t lineEnd = _unparsedBuffer.find("\r\n");
	if (lineEnd == std::string::npos)
		return WAITING_FOR_MORE;
	std::string requestLine = _unparsedBuffer.substr(0, lineEnd);
	setRequestLine(requestLine);
	_unparsedBuffer.erase(0, lineEnd + 2);
	_parsingState = PARSING_HEADERS;
	return RECEIVED_ALL;
}

e_dataProgress	Request::parseHeaderLine()
{
	size_t lineEnd = _unparsedBuffer.find("\r\n");
	if (lineEnd == std::string::npos)
		return WAITING_FOR_MORE;
	std::string headerLine = _unparsedBuffer.substr(0, lineEnd);
	if (headerLine.empty())
		setIfParsingBody();
	else
		addAsHeaderVar(headerLine);
	_unparsedBuffer.erase(0, lineEnd + 2);
	return RECEIVED_ALL;
}

e_dataProgress	Request::parseRequestBody()
{
	if (_contentLength && _unparsedBuffer.size() < _contentLength)
		return WAITING_FOR_MORE;
	else if (!_contentLength && _unparsedBuffer.find("\r\n\r\n") == std::string::npos)
		return WAITING_FOR_MORE;
	_httpBody = _unparsedBuffer.substr(0, _contentLength);
	_unparsedBuffer.erase(0, _contentLength);
	_parsingState = PARSING_DONE;
	return RECEIVED_ALL;
}

e_dataProgress Request::parseChunkedBody()
{
	size_t offset = 0;
	while (true) {
		size_t sizeEnd = _unparsedBuffer.find("\r\n", offset);
		if (sizeEnd == std::string::npos)
			return WAITING_FOR_MORE;

		std::string sizeLine = _unparsedBuffer.substr(offset, sizeEnd - offset);
		size_t chunkSize = 0;
		std::istringstream iss(sizeLine);
		iss >> std::hex >> chunkSize;

		offset = sizeEnd + 2;

		// if chunk size is zero, end of chunked request
		if (chunkSize == 0) {
			if (_unparsedBuffer.size() < offset + 2)
				return WAITING_FOR_MORE;
			offset += 2; // skips final CRLF
			_parsingState = PARSING_DONE;
			return RECEIVED_ALL;
		}

		if (_unparsedBuffer.size() < offset + chunkSize + 2)
			return WAITING_FOR_MORE;

		std::string chunkData = _unparsedBuffer.substr(offset, chunkSize);
		_httpBody += chunkData;

		offset += chunkSize + 2;

		if (offset >= _unparsedBuffer.size())
			return WAITING_FOR_MORE;
	}
}

void	Request::addRequestChunk(std::string chunk)
{
	_unparsedBuffer.append(chunk);
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
				setRoute(findMatchingRoute());
				break;
			}
			case PARSING_BODY:
			{
				if (parseRequestBody() == WAITING_FOR_MORE)
					return;
				break;
			}
			case PARSING_BODY_CHUNKED:
			{
				if (parseChunkedBody() == WAITING_FOR_MORE)
					return;
				break;
			}
			case PARSING_DONE:
				return;
		}
	}
}

void				Request::setIfParsingBody()
{
	if (_method == "HEAD" || _method == "GET")
		_parsingState = PARSING_DONE;
	if (_requestHeaderMap.find("Transfer-Encoding") != _requestHeaderMap.end()
		&& _requestHeaderMap["Transfer-Encoding"] == "chunked")
	{
		_parsingState = PARSING_BODY_CHUNKED;
	}
	else if (_requestHeaderMap.find("Content-Length") != _requestHeaderMap.end())
	{
		_contentLength = atoi(_requestHeaderMap["Content-Length"].c_str());
		_parsingState = PARSING_BODY;
	}
	else
		_parsingState = PARSING_DONE;
}

// Matching route is required at the parsing stage to know if a request is using a valid method
const Route*	Request::findMatchingRoute()
{
	for (size_t i = 0; i < getConf().getRoutes().size(); i++)
	{
		try
		{
			return (getConf().getRoutes()[i].getMatchingRoute(getRequestedURL()));
		}

		catch (const std::runtime_error&)
		{
			continue;
		}
	}

	throw HTTPError(this, 404);
}



