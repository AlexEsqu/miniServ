#include "Response.hpp"
#include <fcntl.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS                                //
///////////////////////////////////////////////////////////////////

Response::Response()
{
}

Response::Response(Request *req)
	: _statusNum(200)
	, _request(req)
	, _byteSent(0)
{
	if (_request->getMethod() == "POST")
		setStatusNum(201);
	if (_request->getMethod() == "GET")
		setStatusNum(200);
}

Response::Response(Request *req, int status)
	: _statusNum(status)
	, _request(req)
	, _byteSent(0)
{
	if (status >= 400)
	{
		createHTTPHeaders();
		return;
	}
}

Response::Response(const Response &copy)
	: _statusNum(copy._statusNum)
	, _request(copy._request)
	, _routedPath(copy._routedPath)
	, _contentType(copy._contentType)
	, _responsePage(copy._responsePage)
	, _byteSent(copy._byteSent)
{
	// std::cout << "Response copy Constructor called" << std::endl;
}

///////////////////////////////////////////////////////////////////
///                        DESTRUCTORS                           //
///////////////////////////////////////////////////////////////////

Response::~Response()
{
	// std::cout << "Response Destructor called" << std::endl;
}

///////////////////////////////////////////////////////////////////
///                        OPERATORS                             //
///////////////////////////////////////////////////////////////////

Response &Response::operator=(const Response &other)
{
	if (this == &other)
		return (*this);

	_statusNum			= other._statusNum;
	_routedPath			= other._routedPath;
	_request			= other._request;
	_contentType		= other._contentType;
	_contentLength		= other._contentLength;
	_responsePage		= other._responsePage;
	return (*this);
}

///////////////////////////////////////////////////////////////////
///                    SETTERS				                     //
///////////////////////////////////////////////////////////////////

void Response::setStatusNum(int number)
{
	this->_statusNum = number;
}

void Response::setContentType(std::string type)
{
	this->_contentType = type;
}

void Response::setContentLength(int length)
{
	this->_contentLength = length;
}

void Response::setContent(std::string content)
{
	_responsePage.clearBuffer();
	_responsePage.writeToBuffer(content);
}

void Response::setRequest(Request *request)
{
	_request = request;
}

bool isDirectory(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

void Response::setRoutedUrl(std::string url)
{
	int i = 0;
	int len = _request->getRoute()->getDefaultFiles().size();
	while (i < len)
	{
		std::ifstream path(("/" + _routedPath).c_str());

		if (url[url.size() - 1] == '/')
			_routedPath = _request->getRoute()->getRootDirectory() + "/" + _request->getRoute()->getDefaultFiles()[i];
		else
			_routedPath = _request->getRoute()->getRootDirectory() + url;
		if (path.is_open() && !isDirectory(_routedPath.c_str()))
		{
			return;
		}
		i++;
	}
	if (i == len)
		_routedPath = "";
}

///////////////////////////////////////////////////////////////////
///                    GETTERS 			                         //
///////////////////////////////////////////////////////////////////


Request *Response::getRequest()
{
	return (_request);
}

std::string Response::getRoutedURL() const
{
	return (_routedPath);
}

int			Response::getStatus() const
{
	return (_statusNum);
}

std::string	Response::getHTTPHeaders() const
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

///////////////////////////////////////////////////////////////////
///                     MEMBER FUNCTIONS                         //
///////////////////////////////////////////////////////////////////

void Response::createHTTPHeaders()
{
	Status status(this->_statusNum);
	if (status.getStatusCode() >= 400) // if its an error
		this->_content = fetchErrorPageContent(status);
	this->_contentLength = this->_content.size();

	std::stringstream header;
	header << _request->getProtocol() << " " << status << "\r\n"
		   << "Content-Type: " << _contentType << "\r\n"
		   << "Content-Length: " << _contentLength << "\r\n"
		   << "Connection: " << (_request->isKeepAlive() ? "keep-alive" : "close") << "\r\n"
		   << "Server: miniServ\r\n";

	std::cout << "Content-Length is " << _contentLength << " + " << header.str().size() << "\n";

	if (_request->getMethod() == "POST")
	{
		header << "Refresh: 0; url=/\r\n";
	}

	header << "\r\n";

	this->_HTTPResponse = header.str() + _content;
}

///////////////////////////////////////////////////////////////////
///                    GETTERS 			                         //
///////////////////////////////////////////////////////////////////

std::string Response::getHTTPResponse() const
{
	return (this->_HTTPResponse);
}

Request *Response::getRequest()
{
	return (_request);
}

std::string Response::getRoutedURL() const
{
	return (_routedPath);
}

int Response::getStatus() const
{
	return (_statusNum);
}

std::string Response::createErrorPageContent(const Status &num)
{
	std::ifstream inputErrorFile;
	std::string errorFile = _request->getRoute()->getRootDirectory() + "/" + "error.html";
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

void		Response::addToContent(const std::string content)
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

	if (_request->getConf().getParamMap().find(errorKey.str()) != _request->getConf().getParamMap().end())
		errorPagePath = _request->getRoute()->getRootDirectory() + _request->getConf().getParamMap().find(errorKey.str())->second;
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
