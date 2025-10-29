#include "Response.hpp"
#include "ClientSocket.hpp"
#include <fcntl.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS                                //
///////////////////////////////////////////////////////////////////

Response::Response(Request& request, Status& status)
	: _request(request)
	, _status(status)
{
}

Response::Response(const Response &copy)
	: _request(copy._request),
	_status(copy._status),
	_routedPath(copy._routedPath),
	_contentType(copy._contentType),
	_responsePage(copy._responsePage),
	_byteSent(copy._byteSent)
{
	// std::cout << "Response copy Constructor called" << std::endl;
}

///////////////////////////////////////////////////////////////////
///                        DESTRUCTORS                           //
///////////////////////////////////////////////////////////////////

Response::~Response()
{
}

///////////////////////////////////////////////////////////////////
///                        OPERATORS                             //
///////////////////////////////////////////////////////////////////

Response &Response::operator=(const Response &other)
{
	if (this == &other)
		return (*this);

	_request = other._request;
	_status = other._status;
	_routedPath = other._routedPath;
	_contentType = other._contentType;
	_contentLength = other._contentLength;
	_responsePage = other._responsePage;
	return (*this);
}

///////////////////////////////////////////////////////////////////
///                    SETTERS				                     //
///////////////////////////////////////////////////////////////////

void	Response::reset()
{
	_routedPath.clear();
	_mapOfHeadersToBeAdded.clear();
	_contentType.clear();
	_contentLength = 0;
	_boundary.clear();
	_HTTPHeaders.clear();
	_responsePage.clearBuffer();
	_byteSent = 0;
}

void	Response::setContentType(std::string type)
{
	this->_contentType = type;
}

void	Response::setContentLength(int length)
{
	this->_contentLength = length;
}

void	Response::setContent(std::string content)
{
	_responsePage.clearBuffer();
	_responsePage.writeToBuffer(content);
}

void	Response::setHeader(std::string key, std::string value)
{
	_mapOfHeadersToBeAdded[key] = value;
}

// sets status code but does not raise error flag in request
void	Response::setStatus(e_status status)
{
	_status.setStatusCode(status);
}

// sets status code and raises error flag in request
void Response::setError(e_status status)
{
	_status.setError(status);
}

void Response::setRoutedUrl(std::string url)
{
	_routedPath = url;
}

bool isDirectory(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

bool isValidPath(std::string &path)
{
	std::ifstream in(path.c_str(), std::ios::binary);
	if (in.is_open() && !isDirectory(path.c_str()))
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////
///                    GETTERS 			                         //
///////////////////////////////////////////////////////////////////

Request&	Response::getRequest()
{
	return (_request);
}

Status&		Response::getStatus()
{
	return (_status);
}

std::string	Response::getRoutedURL() const
{
	return (_routedPath);
}

std::string Response::getHTTPHeaders() const
{
	return (_HTTPHeaders);
}

std::string Response::getHTTPResponse()
{
	std::string result;

	result.append(_HTTPHeaders);

	std::string content = _responsePage.getAllContent();
	result.append(content);
	_byteSent += result.size();

	return result;
}

std::map<size_t, Session>& Response::getSessionMap()
{
	return _request.getSessionMap();
}

std::string Response::getBoundary()
{
	return(_boundary);
}

bool		Response::hasError() const
{
	return (_status.hasError());
}

///////////////////////////////////////////////////////////////////
///                     MEMBER FUNCTIONS                         //
///////////////////////////////////////////////////////////////////

void Response::createHTTPHeaders()
{
	std::map<std::string, std::string>::iterator it;
	std::map<std::string, std::string> cookieMap = getSessionMap()[_request.getSessionId()].getCookies();
	if (getStatus().getStatusCode() >= 400) // if its an error
		setContent(fetchErrorPageContent(getStatus()));
	_contentLength = _responsePage.getBufferSize();
	std::stringstream contentLength;
	contentLength << _contentLength;
	addHttpHeader(_request.getProtocol(), getStatus().getStringStatusCode());
	addHttpHeader("Content-Type", _contentType);
	addHttpHeader("Content-Length", contentLength.str());
	addHttpHeader("Connection", (_request.isKeepAlive() ? "keep-alive" : "close"));
	addHttpHeader("Server", "miniServ");
	if (_request.hasSessionId())
	{
		for (it = cookieMap.begin(); it != cookieMap.end(); it++)
		{
			addHttpHeader("Set-Cookie", it->first + "=" + it->second + "; SameSite=Strict");
		}
	}
	if (_request.getMethodAsString() == "POST")
	{
		_HTTPHeaders += "Refresh: 0; url=/\r\n";
	}
	_HTTPHeaders += "\r\n";
}

void Response::addHttpHeader(std::string key, std::string value)
{
	std::stringstream header;
	header << key << ": " << value << "\r\n";
	_HTTPHeaders += header.str() ;
}

std::string Response::createErrorPageContent(const Status &num)
{
	std::ifstream inputErrorFile;
	std::string errorFile = Router::joinPaths(_request.getConf().getRoot(), "error.html");
	inputErrorFile.open(errorFile.c_str(), std::ifstream::in);
	std::stringstream outputString;
	std::string line;

	if (!inputErrorFile.is_open())
	{
		std::cerr << RED << "Response::createErrorPageContent: Could not open error file: " << errorFile << STOP_COLOR << std::endl;
		return ("");
	}
	/* Could be a better implementation with finding the string
	 in the line instead of matching exactly because if i add anything
	 like an other space in the error.html well it wont find int anymore */
	while (getline(inputErrorFile, line))
	{
		if (line == "    <h1></h1>")
			line.insert(8, num.getStringStatusCode() + " " + num.getStatusMessage());
		if (line == "    <title></title>")
			line.insert(11, num.getStringStatusCode() + num.getStatusMessage());

		outputString << line;
	}
	inputErrorFile.close();
	return (outputString.str());
}

void Response::addToContent(const std::string content)
{
	_responsePage.writeToBuffer(content.c_str(), content.size());
}

std::string Response::fetchErrorPageContent(const Status &num)
{
	std::stringstream errorKey;
	std::ifstream inputErrorFile;
	std::stringstream outputString;
	std::string line;
	std::string errorPagePath;
	errorKey << "error_page_" << num.getStatusCode();

	if (_request.getConf().getParamMap().find(errorKey.str()) != _request.getConf().getParamMap().end())
		errorPagePath = _request.getRoute()->getRootDirectory() + _request.getConf().getParamMap().find(errorKey.str())->second;
	else
		return (createErrorPageContent(num));

	inputErrorFile.open(errorPagePath.c_str());
	if (!inputErrorFile.is_open())
	{
		std::cerr << RED << "fetchErrorPageContent: Could not open error file: " << errorPagePath << STOP_COLOR << std::endl;
		return (createErrorPageContent(num));
	}
	while (getline(inputErrorFile, line))
		outputString << line;
	inputErrorFile.close();
	return (outputString.str());
}
