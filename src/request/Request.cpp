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
	if (_response != NULL)
		delete _response;
	_response = NULL;
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

Response*			Request::getResponse()
{
	return (_response);
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

void			Request::setResponse(Response* response)
{
	_response = response;
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




//------------------------ MEMBER FUNCTIONS ---------------------------------//

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
	_unparsedBuffer.clear();
	setRequestLine(requestLine);

	// erase data used from the chunk
	chunk.erase(0, lineEnd);

	// set parsing state to the next step
	_parsingState = PARSING_HEADERS;

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
	{
		setIfParsingBody();
		setRoute(findMatchingRoute()); // TO DO: make it a check request validity
	}

	// erase data used from the chunk, store the rest
	chunk.erase(0, lineEnd);

	return RECEIVED_ALL;
}

e_dataProgress	Request::parseRequestBody(std::string& chunk)
{
	_requestBodyBuffer.writeToBuffer(_unparsedBuffer + chunk);
	_unparsedBuffer.clear();

	if (_contentLength && _requestBodyBuffer.getBufferSize() < _contentLength)
		return WAITING_FOR_MORE;

	// should only be for chunked request ? TO DO : check
	// else if (!_contentLength && chunk.find("\r\n\r\n") == std::string::npos)
	// 	return WAITING_FOR_MORE;

	_parsingState = PARSING_DONE;

	return RECEIVED_ALL;
}

e_dataProgress Request::parseChunkedBody(std::string& chunk)
{
	_unparsedBuffer.append(chunk);
	chunk.clear();
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
		_requestBodyBuffer.writeToBuffer(chunkData);

		offset += chunkSize + 2;

		if (offset >= _unparsedBuffer.size())
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
			_parsingState = PARSING_DONE;
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
	while (_parsingState != PARSING_DONE)
	{
		switch (_parsingState)
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
	Route*	result = NULL;
	size_t	lenMatch = 0; // to make sure the longest matching url is picked

	for (size_t i = 0; i < getConf().getRoutes().size(); i++)
	{
		std::string	possibleMatch = getConf().getRoutes()[i].getURLPath();

		if (getRequestedURL().find(possibleMatch) != std::string::npos
			&& possibleMatch.size() > lenMatch)
		{
			lenMatch = possibleMatch.size();
			result = const_cast<Route*>(&(getConf().getRoutes()[i]));
		}
	}

	if (result == NULL)
		throw HTTPError(this, 404);

	return result;
}




