#include "Request.hpp"
#include "readability.hpp"
#include "server.hpp"
#include "parsing.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Request::Request(ServerConf &conf, Status &status)
	: _conf(conf),
	  _requestState(EMPTY),
	  _status(status),
	  _method(UNSUPPORTED),
	  _contentLength(0),
	  _cgiStartTime(-1),
	  _sessionId(0)
{
}

Request::Request(const Request &copy)
	: _conf(copy._conf),
	  _requestState(copy._requestState),
	  _status(copy._status),
	  _method(copy.getMethodCode()),
	  _contentLength(copy._contentLength),
	  _cgiStartTime(copy._cgiStartTime),
	  _sessionId(copy.getSessionId())
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
	if (this != &other)
	{
		_unparsedBuffer = other._unparsedBuffer;
		_methodAsString = other._methodAsString;
		_protocol = other._protocol;
		_URI = other._URI;
		_contentLength = other._contentLength;
		_requestHeaderMap = other._requestHeaderMap;
		_route = other._route;
		_requestBodyBuffer = other._requestBodyBuffer;
		_status = other._status;
		_sessionId = other._sessionId;
		_cgiStartTime = other._cgiStartTime;
		_cgiForkPid = other._cgiForkPid;
	}

	return *this;
}

//---------------------------- GUETTERS -------------------------------------//

std::string Request::getMethodAsString() const
{
	return _methodAsString;
}

e_methods Request::getMethodCode() const
{
	return _method;
}

std::string Request::getProtocol() const
{
	return _protocol;
}

std::string Request::getRequestedURL() const
{
	return _URI;
}

std::map<std::string, std::string> &Request::getAdditionalHeaderInfo()
{
	return _requestHeaderMap;
}

void Request::setContentType(std::string string)
{
	_contentType = string;
}

std::string Request::getContentType() const
{
	return (_contentType);
}

const ServerConf &Request::getConf() const
{
	return _conf;
}

std::map<size_t, Session> &Request::getSessionMap()
{
	return _conf.getSessionMap();
}

Status &Request::getStatus()
{
	return _status;
}

int Request::getParsingState() const
{
	return _requestState;
}

const Route *Request::getRoute() const
{
	return (_route);
}

bool Request::hasSessionId() const
{
	return (_sessionId != 0);
}

size_t Request::getSessionId() const
{
	return (_sessionId);
}

std::string Request::getStringSessionId() const
{
	std::stringstream temp;
	temp << _sessionId;
	return (temp.str());
}

std::string Request::getBody() const
{
	if (_requestBodyBuffer.isUsingFile())
	{
		std::string content = _requestBodyBuffer.getAllContent();
		return content;
	}
	else
	{
		std::string content = _requestBodyBuffer.getMemoryBuffer();
		return content;
	}
}

int Request::getCgiPipe() const
{
	return (_readingEndOfCGIPipe);
}

int Request::getCgiForkPid() const
{
	return (_cgiForkPid);
}

std::string Request::getCgiParam() const
{
	return (_paramCGI);
}

time_t Request::getCgiStartTime() const
{
	return _cgiStartTime;
}

size_t		Request::getContentLength() const
{
	return (_contentLength);
}

Buffer&		Request::getBodyBuffer()
{
	return (_requestBodyBuffer);
}

std::istream &Request::getStreamFromBodyBuffer()
{
	return (_requestBodyBuffer.getStream());
}

bool Request::isKeepAlive()
{
	if (_requestHeaderMap.find("connection") != _requestHeaderMap.end())
		return (_requestHeaderMap["connection"] == "keep-alive");
	return false;
}

bool Request::hasError() const
{
	return _status.hasError();
}

//----------------------- SETTERS -----------------------------------//

void Request::reset()
{
	_methodAsString.clear();
	_method = UNSUPPORTED;
	_protocol.clear();
	_URI.clear();
	_requestHeaderMap.clear();
	_isChunked = false;
	_contentLength = 0;
	_contentType.clear();
	_unparsedBuffer.clear();
	_requestBodyBuffer.clearBuffer();
	_readingEndOfCGIPipe = -1;
	_route = NULL;
	_paramCGI.clear();
	_cgiStartTime = -1;
	_sessionId = 0;
	_requestState = EMPTY;
}

void Request::setMethod(std::string &method)
{
	_methodAsString = method;
	if (_methodAsString == "GET")
		_method = GET;
	// else if (_methodAsString == "PUT")
	// 	_method = PUT;
	// else if (_methodAsString == "HEAD")
	// 	_method = HEAD;
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
void Request::setURI(std::string &URI)
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

void Request::setProtocol(std::string &protocol)
{
	_protocol = protocol;
	if (_protocol != "HTTP/1.1")
		setError(HTTP_VERSION_NOT_SUPPORTED);
}

void	Request::setContentLength(std::string &lengthAsStr)
{
	size_t	contentLength = std::atoi(lengthAsStr.c_str());
	size_t	max = getConf().getMaxSizeClientRequestBody();

	if (contentLength > max)
	{
		_contentLength = getConf().getMaxSizeClientRequestBody();
		setError(PAYLOAD_TOO_LARGE);
		_requestState = PARSING_DONE;
	}
	else
		_contentLength = contentLength;
}

void	Request::setKeepAlive(bool value)
{
	if (value)
		_requestHeaderMap["connection"] = "keep-alive";
	else
		_requestHeaderMap["connection"] = "close";
}

void Request::setRoute(const Route *route)
{
	_route = route;
}

void Request::setParsingState(e_requestState requestState)
{
	_requestState = requestState;
}

// Valid request line (1st line of a HTTP request) must have the format:
//    Method SP Request-URI SP HTTP-Version CRLF
void Request::setRequestLine(std::string &requestLine)
{
	std::vector<std::string> splitRequestLine = split(requestLine, ' ');
	if (splitRequestLine.size() != 3)
	{
		setError(BAD_REQUEST);
		return;
	}
	setMethod(trim(splitRequestLine[0]));
	setURI(trim(splitRequestLine[1]));
	setProtocol(trim(splitRequestLine[2]));
	std::cout << _methodAsString << " " << _URI << " ";
}
// Cookie: a=valeur
// Cookie: b=autreValeur

void Request::addAsHeaderVar(std::string &keyValueString)
{
	size_t equalPos = keyValueString.find(':');
	if (equalPos != std::string::npos)
	{
		std::string key = keyValueString.substr(0, equalPos);
		std::string value = keyValueString.substr(equalPos + 1);
		key = trim(key);
		value = trim(value);
		if (key == "Cookie")
		{
			if (value.find("session_id") != std::string::npos) // if session_id is in cookie
			{
				_sessionId = std::atoi(value.substr(value.find('=') + 1).c_str());
				getSessionMap()[_sessionId].addCookie(value);
				return;
			}
		}
		else if (hasSessionId() == false)
		{
			// assign a pseudo random number to session_id if it doesn't exist
			_sessionId = Session::generatePseudoRandomNumber();
			getSessionMap().insert(std::pair<size_t, Session>(_sessionId, Session(_sessionId)));
			std::stringstream sessionCookie;
			sessionCookie << "session_id=" << _sessionId;
			getSessionMap()[_sessionId].addCookie(sessionCookie.str());
			return;
		}

		strToLower(key);
		strToLower(value);
		_requestHeaderMap[key] = value;

		if (key == "content-type")
			setContentType(value);

		if (key == "content-length")
			setContentLength(value);

		verboseLog("Header: [" + key + "] = [" + value + "]");
	}
}

// sets the Status object to error code and raises the error flag
void Request::setError(e_status statusCode)
{
	_status.setStatusCode(statusCode);
	std::ostringstream oss;
	oss << "Setting error at " << statusCode;
	verboseLog(oss.str());
}

// sets the Status object to error code WITHOUT raising the error flag
void Request::setStatus(e_status statusCode)
{
	_status.setStatusCode(statusCode);
}

void Request::setCgiPipe(int pipeFd)
{
	_readingEndOfCGIPipe = pipeFd;
}

void Request::setCgiForkPid(int forkPid)
{
	_cgiForkPid = forkPid;
}

void Request::setCgiStartTime()
{
	_cgiStartTime = std::time(0);
	if (_cgiStartTime < 0)
		throw std::runtime_error("Failed call to std::time()");
}

//----------------------- INTERNAL FUNCTIONS -----------------------------------//

//------------------------ MEMBER FUNCTIONS ---------------------------------//

void Request::checkMethodIsAllowed()
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

void Request::validateRequestLine()
{
	_route = Router::findMatchingRoute(_URI, _conf);
	checkMethodIsAllowed();
}

e_dataProgress Request::parseRequestLine(std::string &chunk)
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

e_dataProgress Request::parseHeaderLine(std::string &chunk)
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
	std::string headerLine = _unparsedBuffer + chunk.substr(0, lineEnd);
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
void Request::addRequestChunk(std::string chunk)
{
	if (_requestState == EMPTY)
		_requestState = PARSING_REQUEST_LINE;

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
void Request::setIfAssemblingBody()
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

	if (_requestHeaderMap.find("transfer-encoding") != _requestHeaderMap.end() && _requestHeaderMap["transfer-encoding"] == "chunked")
	{
		_requestState = PARSING_BODY;
		_isChunked = true;
	}
	else if (_contentLength)
	{
		_requestState = PARSING_BODY;
		_isChunked = false;
	}
	else
		_requestState = PARSING_DONE;
}

// Making sure the body has been received in full, whether chunked or with Content-Length
e_dataProgress Request::assembleBody(std::string &chunk)
{
	if (_isChunked)
		return assembleChunkedBody(chunk);
	else
		return assembleUnChunkedBody(chunk);
}

// made for many small chunks which need to be parsed, so using the unparsedbuffer
// for string manipulation before writing to the Buffer object
e_dataProgress Request::assembleChunkedBody(std::string &chunk)
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

		size_t	remainderToRead = getContentLength() - _requestBodyBuffer.getBufferSize();
		size_t	newSize = _requestBodyBuffer.getBufferSize() + chunkData.size();
		if (newSize > getConf().getMaxSizeClientRequestBody())
		{
			setError(PAYLOAD_TOO_LARGE);
			_requestState = PARSING_DONE;
			return RECEIVED_ALL;
		}
		_requestBodyBuffer.writeToBuffer(chunkData.substr(0, remainderToRead));

		// move on the the next chunk which may be in the same buffer
		offset += chunkSize + 2;
		if (offset >= _unparsedBuffer.size())
			return WAITING_FOR_MORE;
	}
}

// Made for large single request, so making use of the temporary file buffer object
e_dataProgress Request::assembleUnChunkedBody(std::string &chunk)
{
	size_t	remainderToRead = getContentLength() - _requestBodyBuffer.getBufferSize();

	// copy all leftover from the parsing into the buffer
	if (!_unparsedBuffer.empty())
	{
		_requestBodyBuffer.writeToBuffer(_unparsedBuffer.substr(0, remainderToRead));
		_unparsedBuffer.clear();
	}

	remainderToRead = getContentLength() - _requestBodyBuffer.getBufferSize();
	size_t	newSize = _requestBodyBuffer.getBufferSize() + _requestBodyBuffer.getBufferSize();
	if (newSize > getConf().getMaxSizeClientRequestBody())
	{
		setError(PAYLOAD_TOO_LARGE);
		_requestState = PARSING_DONE;
		return RECEIVED_ALL;
	}

	_requestBodyBuffer.writeToBuffer(chunk.substr(0, remainderToRead));
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
