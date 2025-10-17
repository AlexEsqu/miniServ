#include "Request.hpp"
#include "readability.hpp"
#include "server.hpp"
#include "parsing.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

// initializes a new request with a first chunk of request content
// the object starts parsing as it receives information
// the status is initialized empty, so set at 200 / SUCCESS
Request::Request(const ServerConf& conf, std::string requestChunk)
	: _conf(conf)
	, _requestState(PARSING_REQUEST_LINE)
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
	if (_response != NULL)
		delete _response;
	_response = NULL;
}

//---------------------------- OPERATORS ------------------------------------//

Request &Request::operator=(const Request &other)
{
	if (this != &other) {
		_unparsedHeaderBuffer	= other._unparsedHeaderBuffer;
		_methodAsString			= other._methodAsString;
		_protocol				= other._protocol;
		_URI					= other._URI;
		_requestHeaderMap		= other._requestHeaderMap;
		_route					= other._route;
		_requestBodyBuffer		= other._requestBodyBuffer;
	}

	return *this;
}

//---------------------------- GUETTERS -------------------------------------//

std::string			Request::getMethodAsString() const
{
	return _methodAsString;
}

e_methods			Request::getMethodCode() const
{
	return _method;
}

std::string			Request::getProtocol() const
{
	return _protocol;
}

std::string			Request::getRequestedURL() const
{
	return _URI;
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
	return _requestState;
}

const Route*		Request::getRoute() const
{
	return (_route);
}

Response*			Request::getResponse()
{
	return (_response);
}

std::string			Request::getBody() const
{
	std::cout << "=== REQUEST BODY DEBUG ===" << std::endl;
	std::cout << "Buffer using file: " << _requestBodyBuffer.isUsingFile() << std::endl;
	std::cout << "Buffer size: " << _requestBodyBuffer.getBufferSize() << std::endl;

	if (_requestBodyBuffer.isUsingFile()) {
		std::cout << "Reading from file buffer..." << std::endl;
		std::string content = _requestBodyBuffer.getAllContent();
		std::cout << "File content size: " << content.size() << std::endl;
		return content;
	} else {
		std::cout << "Reading from memory buffer..." << std::endl;
		std::string content = _requestBodyBuffer.getMemoryBuffer();
		std::cout << "Memory content size: " << content.size() << std::endl;
		return content;
	}
}

bool				Request::isKeepAlive()
{
	if (_requestHeaderMap.find("Connection") != _requestHeaderMap.end())
		return (_requestHeaderMap["Connection"] == "keep-alive");
	return false;
}

bool				Request::hasError() const
{
	return _status.hasError();
}

//----------------------- SETTERS -----------------------------------//

void	Request::setMethod(std::string &method)
{
	_methodAsString = method;
	if (_methodAsString == "GET")
		_method = GET;
	else if (_methodAsString == "PUT")
		_method = PUT;
	else if (_methodAsString == "HEAD")
		_method = HEAD;
	else if (_methodAsString == "DELETE")
		_method = DELETE;
	else if (_methodAsString == "POST")
		_method = POST;
	else
		_method = UNSUPPORTED;
}

// extracts the ?key=value CGI param from the URI, storing them in another string
void	Request::setURI(std::string &URI)
{
	size_t queryPos = URI.find('?');
	if (queryPos != std::string::npos)
	{
		_paramCGI = URI.substr(queryPos + 1, URI.size());
		_URI = URI.substr(0, queryPos);
	}
	else
		_URI = URI;
}

void	Request::setProtocol(std::string &protocol)
{
	_protocol = protocol;
}

void	Request::setRoute(const Route* route)
{
	_route = route;
}

void	Request::setResponse(Response* response)
{
	_response = response;
}

void	Request::setParsingState(e_requestState requestState)
{
	_requestState = requestState;
}

// Valid request line (1st line of a HTTP request) must have the format:
//    Method SP Request-URI SP HTTP-Version CRLF
void	Request::setRequestLine(std::string &requestLine)
{
	std::vector<std::string> splitRequestLine = split(requestLine, ' ');
	if (splitRequestLine.size() != 3)
	{
		std::cout << "Invalid line format";
		setError(400);
		return;
	}
	setMethod(trim(splitRequestLine[0]));
	setURI(trim(splitRequestLine[1]));
	setProtocol(trim(splitRequestLine[2]));



	std::cout << _methodAsString << " " << _URI << " ";
}

void	Request::addAsHeaderVar(std::string &keyValueString)
{
	size_t	equalPos = keyValueString.find(':');

	if (equalPos != std::string::npos) {
		std::string key = keyValueString.substr(0, equalPos);
		std::string value = keyValueString.substr(equalPos + 1);
		key = trim(key);
		value = trim(value);
		strToLower(key);
		strToLower(value);
		_requestHeaderMap[key] = value;

		#ifdef DEBUG
			std::cout << "adding Header var [" << key << "] = [" << value << "]\n";
		#endif
	}
}

// sets the Status object to error code and raises the error flag
void	Request::setError(unsigned int statusCode)
{
	_status.setStatusCode(statusCode);
	_hasError = true;
	#ifdef DEBUG
		std::cerr << "Setting error at " << statusCode << "\n";
	#endif
}

// sets the Status object to error code WITHOUT raising the error flag
void	Request::setStatus(unsigned int statusCode)
{
	_status.setStatusCode(statusCode);
}



//----------------------- INTERNAL FUNCTIONS -----------------------------------//




//------------------------ MEMBER FUNCTIONS ---------------------------------//

void			Request::checkMethodIsAllowed()
{
	if (_hasError)
		return;

	if (_methodAsString.empty() || _method == UNSUPPORTED)
		setError(405);

	if (!_route->isAllowedMethod(_methodAsString))
		setError(405);
}

void			Request::validateRequestLine()
{
	_route = findMatchingRoute();
	checkMethodIsAllowed();
}

e_dataProgress	Request::parseRequestLine(std::string& chunk)
{
	// store chunk and return to epoll if the chunk does not contain
	// the end of the request line
	size_t lineEnd = chunk.find("\r\n");
	if (lineEnd == std::string::npos)
	{
		_unparsedHeaderBuffer.append(chunk);
		return WAITING_FOR_MORE;
	}

	// create request line out of chunk and possible unparsed leftover
	std::string requestLine = _unparsedHeaderBuffer + chunk.substr(0, lineEnd);
	setRequestLine(requestLine);
	validateRequestLine();

	// erase data used from the buffer, from the chunk, and the uneeded \r\n
	_unparsedHeaderBuffer.clear();
	chunk.erase(0, lineEnd + 2);

	// set parsing state to the next step
	_requestState = PARSING_HEADERS;

	// indicate the request line has been received in full
	return RECEIVED_ALL;
}

e_dataProgress	Request::parseHeaderLine(std::string& chunk)
{
	// store chunk and return to epoll if the chunk does not contain
	// the end of the header line
	size_t lineEnd = chunk.find("\r\n");
	if (lineEnd == std::string::npos)
	{
		_unparsedHeaderBuffer.append(chunk);
		return WAITING_FOR_MORE;
	}

	// create request line out of chunk and possible unparsed leftover
	std::string headerLine =_unparsedHeaderBuffer + chunk.substr(0, lineEnd);
	_unparsedHeaderBuffer.clear();

	// either add the header line as variable to the header map of variable
	if (!headerLine.empty())
		addAsHeaderVar(headerLine);
	// or if it's empty, I reached the end of the headers and can set if
	// parsing the body is necessary, which will update the parsing state
	else
		setIfParsingBody();

	// erase data used from the chunk, store the rest
	chunk.erase(0, lineEnd + 2);

	return RECEIVED_ALL;
}

e_dataProgress	Request::parseRequestBody(std::string& chunk)
{
	// copy all leftover from the parsing into the buffer
	_requestBodyBuffer.writeToBuffer(_unparsedHeaderBuffer + chunk);
	_unparsedHeaderBuffer.clear();
	chunk.clear();

	if (_isChunked)
		return parseChunkedBody();

	else if (_requestBodyBuffer.getBufferSize() < _contentLength)
		return WAITING_FOR_MORE;

	_requestState = PARSING_DONE;

	return RECEIVED_ALL;
}

e_dataProgress Request::parseChunkedBody()
{
	size_t offset = 0;
	while (true) {
		size_t sizeEnd = _unparsedHeaderBuffer.find("\r\n", offset);
		if (sizeEnd == std::string::npos)
			return WAITING_FOR_MORE;

		std::string sizeLine = _unparsedHeaderBuffer.substr(offset, sizeEnd - offset);
		size_t chunkSize = 0;
		std::istringstream iss(sizeLine);
		iss >> std::hex >> chunkSize;

		offset = sizeEnd + 2;

		// if chunk size is zero, end of chunked request
		if (chunkSize == 0) {
			if (_unparsedHeaderBuffer.size() < offset + 2)
				return WAITING_FOR_MORE;
			offset += 2; // skips final CRLF
			_requestState = PARSING_DONE;
			return RECEIVED_ALL;
		}

		if (_unparsedHeaderBuffer.size() < offset + chunkSize + 2)
			return WAITING_FOR_MORE;

		std::string chunkData = _unparsedHeaderBuffer.substr(offset, chunkSize);
		_requestBodyBuffer.writeToBuffer(chunkData);

		offset += chunkSize + 2;

		if (offset >= _unparsedHeaderBuffer.size())
			return WAITING_FOR_MORE;
	}
}

// For every chunk of data added to the request, parsing continues from last state
// and returns if the current parsed item (header, body...) is not finished
void	Request::addRequestChunk(std::string chunk)
{
	while (_requestState != PARSING_DONE)
	{
		switch (_requestState)
		{
			case PARSING_REQUEST_LINE:
			{
				if (parseRequestLine(chunk) == WAITING_FOR_MORE)
					return;
				break;
			}
			case PARSING_HEADERS:
			{
				if (parseHeaderLine(chunk) == WAITING_FOR_MORE)
					return;
				break;
			}
			case PARSING_BODY:
			{
				if (parseRequestBody(chunk) == WAITING_FOR_MORE)
					return;
				break;
			}
			default:
				return;
		}
	}
}

// It is not necessary to parse a body if the request is a GET or HEAD
// even when a body should be parsed, it is only up to content length
// unless the request is specifically marked as chunked by transfer encoding
void				Request::setIfParsingBody()
{
	// for certain methods and if an error is found in the headers, no need to parse body
	if (_method == HEAD || _method == GET || _hasError)
		_requestState = PARSING_DONE;
	if (_requestHeaderMap.find("transfer-encoding") != _requestHeaderMap.end()
		&& _requestHeaderMap["transfer-encoding"] == "chunked")
	{
		_requestState = PARSING_BODY;
		_isChunked = true;
	}
	else if (_requestHeaderMap.find("content-length") != _requestHeaderMap.end())
	{
		_contentLength = atoi(_requestHeaderMap["content-length"].c_str());
		_requestState = PARSING_BODY;
		_isChunked = false;
	}
	else
		_requestState = PARSING_DONE;
}

// Matching route is required at the parsing stage to know if a request is using a valid method
const Route*	Request::findMatchingRoute()
{
	const Route* result = NULL;
	size_t lenMatch = 0;

	std::string requestPath = getRequestedURL();

	for (size_t i = 0; i < getConf().getRoutes().size(); i++)
	{
		const Route& route = getConf().getRoutes()[i];

		if (route.isPathMatch(requestPath))
		{
			std::string routePath = route.getURLPath();
			if (routePath.size() > lenMatch)
			{
				lenMatch = routePath.size();
				result = &route;
			}
		}
	}

	if (result == NULL)
	{
		std::cout << RED << "404: No route for: " + requestPath << STOP_COLOR;
		setError(404);
		return NULL;
	}
	return result;
}




