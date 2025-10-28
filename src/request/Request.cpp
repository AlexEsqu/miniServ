#include "Request.hpp"
#include "readability.hpp"
#include "server.hpp"
#include "parsing.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

// initializes a new request with a first chunk of request content
// the object starts parsing as it receives information
// the status is initialized empty, so set at 200 / SUCCESS
Request::Request(const ServerConf& conf, Status& status, std::string requestChunk)
	: _conf(conf)
	, _requestState(PARSING_REQUEST_LINE)
	, _status(status)
{
	addRequestChunk(requestChunk);
}

Request::Request(const Request &copy)
	: _conf(copy._conf)
	, _status(copy._status)
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
		_unparsedBuffer	= other._unparsedBuffer;
		_methodAsString			= other._methodAsString;
		_protocol				= other._protocol;
		_URI					= other._URI;
		_requestHeaderMap		= other._requestHeaderMap;
		_route					= other._route;
		_requestBodyBuffer		= other._requestBodyBuffer;
		_status					= other._status;
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

void				Request::setContentType(std::string string)
{
	_contentType = string;
}

std::string Request::getContentType() const
{
	return(_contentType);
}

const ServerConf&	Request::getConf() const
{
	return _conf;
}

Status&		Request::getStatus()
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

std::string			Request::getBody() const
{
	// std::cout << "=== REQUEST BODY DEBUG ===" << std::endl;
	// std::cout << "Buffer using file: " << _requestBodyBuffer.isUsingFile() << std::endl;
	// std::cout << "Buffer size: " << _requestBodyBuffer.getBufferSize() << std::endl;

	if (_requestBodyBuffer.isUsingFile()) {
		// std::cout << "Reading from file buffer..." << std::endl;
		std::string content = _requestBodyBuffer.getAllContent();
		// std::cout << "File content size: " << content.size() << std::endl;
		return content;
	} else {
		// std::cout << "Reading from memory buffer..." << std::endl;
		std::string content = _requestBodyBuffer.getMemoryBuffer();
		// std::cout << "Memory content size: " << content.size() << std::endl;
		return content;
	}
}

int			Request::getCgiPipe() const
{
	return (_readingEndOfCGIPipe);
}

std::istream&	Request::getStreamFromBodyBuffer()
{
	return (_requestBodyBuffer.getStream());
}

bool				Request::isKeepAlive()
{
	if (_requestHeaderMap.find("connection") != _requestHeaderMap.end())
		return (_requestHeaderMap["connection"] == "keep-alive");
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
	{
		_method = UNSUPPORTED;
		setError(METHOD_NOT_ALLOWED);
	}
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
	if (_protocol != "HTTP/1.1")
		setError(HTTP_VERSION_NOT_SUPPORTED);
}

void	Request::setRoute(const Route* route)
{
	_route = route;
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
		setError(BAD_REQUEST);
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

		if (key == "content-type")
			setContentType(value);

		verboseLog("Header: [" + key + "] = [" + value + "]");
	}
}

// sets the Status object to error code and raises the error flag
void	Request::setError(e_status statusCode)
{
	_status.setStatusCode(statusCode);
	std::ostringstream oss;
	oss << "Setting error at " << statusCode;
	verboseLog(oss.str());
}

// sets the Status object to error code WITHOUT raising the error flag
void	Request::setStatus(e_status statusCode)
{
	_status.setStatusCode(statusCode);
}

void	Request::setCgiPipe(int pipeFd)
{
	_readingEndOfCGIPipe = pipeFd;
}


//----------------------- INTERNAL FUNCTIONS -----------------------------------//




//------------------------ MEMBER FUNCTIONS ---------------------------------//

void			Request::checkMethodIsAllowed()
{
	if (hasError())
		return;

	if (!_route)
		setError(METHOD_NOT_ALLOWED); // probably tested earlier but for unit test

	else if (_methodAsString.empty() || _method == UNSUPPORTED)
		setError(METHOD_NOT_ALLOWED);

	else if (!_route->isAllowedMethod(_methodAsString))
		setError(METHOD_NOT_ALLOWED);
}

void			Request::validateRequestLine()
{
	_route = Router::findMatchingRoute(_URI, _conf);
	checkMethodIsAllowed();
}

e_dataProgress	Request::parseRequestLine(std::string& chunk)
{
	// store chunk and return to epoll if the chunk does not contain
	// the end of the request line
	size_t lineEnd = chunk.find("\r\n");
	if (lineEnd == std::string::npos)
	{
		_unparsedBuffer.append(chunk);
		return WAITING_FOR_MORE;
	}

	// create request line out of chunk and possible unparsed leftover
	std::string requestLine = _unparsedBuffer + chunk.substr(0, lineEnd);
	setRequestLine(requestLine);
	validateRequestLine();

	// erase data used from the buffer, from the chunk, and the uneeded \r\n
	_unparsedBuffer.clear();
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
		_unparsedBuffer.append(chunk);
		return WAITING_FOR_MORE;
	}

	// create request line out of chunk and possible unparsed leftover
	std::string headerLine =_unparsedBuffer + chunk.substr(0, lineEnd);
	_unparsedBuffer.clear();

	// either add the header line as variable to the header map of variable
	if (!headerLine.empty())
		addAsHeaderVar(headerLine);
	// or if it's empty, I reached the end of the headers and can set if
	// parsing the body is necessary, which will update the parsing state
	else
		setIfAssemblingBody();

	// erase data used from the chunk, store the rest
	chunk.erase(0, lineEnd + 2);

	return RECEIVED_ALL;
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
				if (assembleBody(chunk) == WAITING_FOR_MORE)
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
void				Request::setIfAssemblingBody()
{
	// for certain methods and if an error is found in the headers, no need to parse body
	if (_method == HEAD || _method == GET || _method == DELETE || hasError())
	{
		_requestState = PARSING_DONE;
		return;
	}

	// we do not allow POST or PUT without content type
	if (_contentType.empty())
	{
		setError(UNSUPPORTED_MEDIA_TYPE);
		_requestState = PARSING_DONE;
	}

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

// Making sure the body has been received in full, whether chunked or with Content-Length
e_dataProgress	Request::assembleBody(std::string& chunk)
{
	if (_isChunked)
		return assembleChunkedBody(chunk);
	else
		return assembleUnChunkedBody(chunk);
}

// made for many small chunks which need to be parsed, so using the unparsedbuffer
// for string manipulation before writing to the Buffer object
e_dataProgress Request::assembleChunkedBody(std::string& chunk)
{
	// append to chunked body any possible leftover from the header parsing
	_unparsedBuffer.append(chunk);
	chunk.clear();
	size_t offset = 0;
	while (true)
	{
		// identify a chunk in what has been received, else wait for a full chunk
		size_t sizeEnd = _unparsedBuffer.find("\r\n", offset);
		if (sizeEnd == std::string::npos)
			return WAITING_FOR_MORE;

		// get the size of the identified chunk
		std::string sizeLine = _unparsedBuffer.substr(offset, sizeEnd - offset);
		size_t chunkSize = 0;
		std::istringstream iss(sizeLine);
		iss >> std::hex >> chunkSize;
		// std::cout << "chunksize is [" << chunkSize << "]\n";

		offset = sizeEnd + 2;

		// if chunk size is zero, end of chunked request
		if (chunkSize == 0)
		{
			_requestState = PARSING_DONE;
			return RECEIVED_ALL;
		}

		// the identified chunk has not been received in full yet
		if (_unparsedBuffer.size() < offset + chunkSize)
			return WAITING_FOR_MORE;

		// extracting a chunk into the Buffer
		std::string chunkData = _unparsedBuffer.substr(offset, chunkSize - 2);
		// std::cout << "chunk data [" << chunkData << "]\n";
		_requestBodyBuffer.writeToBuffer(chunkData);

		// move on the the next chunk which may be in the same buffer
		offset += chunkSize + 2;
		if (offset >= _unparsedBuffer.size())
			return WAITING_FOR_MORE;
	}
}

// Made for large single request, so making use of the temporary file buffer object
e_dataProgress Request::assembleUnChunkedBody(std::string& chunk)
{
	// copy all leftover from the parsing into the buffer
	if (!_unparsedBuffer.empty())
	{
		_requestBodyBuffer.writeToBuffer(_unparsedBuffer);
		_unparsedBuffer.clear();
	}
	_requestBodyBuffer.writeToBuffer(chunk);
	chunk.clear();

	// check received content is correct length or wait for more
	// std::cout << "buffer size is [" << _requestBodyBuffer.getBufferSize() << "]\n";
	if (_requestBodyBuffer.getBufferSize() < _contentLength)
		return WAITING_FOR_MORE;
	else
	{
		_requestState = PARSING_DONE;
		return RECEIVED_ALL;
	}
}
