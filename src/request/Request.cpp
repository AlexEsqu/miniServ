#include "Request.hpp"
#include "readability.hpp"
#include "server.hpp"
#include "parsing.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Request::Request(const ServerConf& conf, std::string requestChunk)
	: _conf(conf)
	, _requestState(PARSING_REQUEST_LINE)
	, _status(200)
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
		_unparsedHeaderBuffer = other._unparsedHeaderBuffer;
		_methodAsString = other._methodAsString;
		_protocol = other._protocol;
		_URI = other._URI;
		_requestHeaderMap = other._requestHeaderMap;
		_route = other._route;
		_requestBodyBuffer = other._requestBodyBuffer;
	}

	return *this;
}

//---------------------------- GUETTERS -------------------------------------//

std::string		Request::getMethod() const
{
	return _methodAsString;
}

std::string		Request::getProtocol() const
{
	return _protocol;
}

std::string		Request::getRequestedURL() const
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

void	Request::setURI(std::string &URI)
{
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
		throw HTTPError(this, 400);
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

//----------------------- INTERNAL FUNCTIONS -----------------------------------//




//------------------------ MEMBER FUNCTIONS ---------------------------------//

void			Request::checkMethodIsAllowed()
{
	if (_methodAsString.empty() || _method == UNSUPPORTED)
		throw HTTPError(this, METHOD_NOT_ALLOWED);

	if (!_route->isAllowedMethod(_methodAsString))
		throw HTTPError(this, METHOD_NOT_ALLOWED);
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
	_unparsedHeaderBuffer.clear();

	setRequestLine(requestLine);

	// erase data used from the chunk and unneeded \r\n
	chunk.erase(0, lineEnd + 2);

	// set parsing state to the next step
	_requestState = PARSING_HEADERS;

	// checks the request line is valid
	validateRequestLine();

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

	if (_contentLength && _requestBodyBuffer.getBufferSize() < _contentLength)
		return WAITING_FOR_MORE;

	// should only be for chunked request ? TO DO : check
	// else if (!_contentLength && chunk.find("\r\n\r\n") == std::string::npos)
	// 	return WAITING_FOR_MORE;

	_requestState = PARSING_DONE;

	return RECEIVED_ALL;
}

e_dataProgress Request::parseChunkedBody(std::string& chunk)
{
	_requestBodyBuffer.writeToBuffer(_unparsedHeaderBuffer + chunk);
	_unparsedHeaderBuffer.clear();
	chunk.clear();

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

e_dataProgress		Request::parseChunkedBody(std::istream& in)
{
	while (true) {

		// checks the size of the chunk has been received
		std::string sizeLine;
		if (!std::getline(in, sizeLine, '\n'))
			return WAITING_FOR_MORE;

		// atoi the size of the chunk
		size_t chunkSize = 0;
		std::istringstream iss(sizeLine);
		iss >> std::hex >> chunkSize;

		// if the chunk size is zero, chunked parsing is done, returning
		if (chunkSize == 0) {
			_requestState = PARSING_DONE;
			return RECEIVED_ALL;
		}

		// else, extracting the raw data up to chunkSize, or waiting for more
		std::string chunkData(chunkSize, '\0');
		in.read(&chunkData[0], chunkSize);
		if (in.gcount() < static_cast<std::streamsize>(chunkSize))
			return WAITING_FOR_MORE;

		// if the whole chunk has been extracted, writing it to buffer
		_requestBodyBuffer.writeToBuffer(chunkData);

		// read and discard trailing CRLF after chunk data for socket ease
		char crlf[2];
		in.read(crlf, 2);

		// might be signed the request is malformed or incomplete
		if (in.gcount() < 2 || std::string(crlf, 2) != "\r\n")
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
			case PARSING_BODY_CHUNKED:
			{
				if (parseChunkedBody(chunk) == WAITING_FOR_MORE)
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
	if (_methodAsString == "HEAD" || _methodAsString == "GET")
		_requestState = PARSING_DONE;
	if (_requestHeaderMap.find("transfer-encoding") != _requestHeaderMap.end()
		&& _requestHeaderMap["transfer-encoding"] == "chunked")
	{
		_requestState = PARSING_BODY_CHUNKED;
	}
	else if (_requestHeaderMap.find("content-length") != _requestHeaderMap.end())
	{
		_contentLength = atoi(_requestHeaderMap["content-length"].c_str());
		_requestState = PARSING_BODY;
	}
	else
		_requestState = PARSING_DONE;
}

// extracts the ?key=value CGI param from the URI, storing them in another string
void	Request::extractIfCGIParam()
{
	size_t queryPos = _URI.find('?');
	if (queryPos != std::string::npos) {
		_paramCGI = _URI.substr(queryPos + 1, _URI.size());
		_URI = _URI.substr(0, queryPos);
	}
}

// Matching route is required at the parsing stage to know if a request is using a valid method
const Route*	Request::findMatchingRoute()
{
	const Route* result = NULL;
	size_t lenMatch = 0;

	extractIfCGIParam();

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
		throw HTTPError(this, 404);

	return result;
}




