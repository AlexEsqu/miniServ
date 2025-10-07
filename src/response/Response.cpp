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
{
	if (_request->getMethod() == "POST")
		setStatusNum(201);
	if (_request->getMethod() == "GET")
		setStatusNum(200);
}

Response::Response(Request *req, int status)
	: _statusNum(status)
	, _request(req)
{
	if (status >= 400)
	{
		AddHTTPHeaders();
		return;
	}
}

Response::Response(const Response &copy)
	: _statusNum(copy._statusNum)
	, _request(copy._request)
	, _routedPath(copy._routedPath)
	, _contentType(copy._contentType)
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

Response&	Response::operator=(const Response &other)
{
	if (this == &other)
		return (*this);

	_statusNum			= other._statusNum;
	_routedPath			= other._routedPath;
	_request			= other._request;
	_contentType		= other._contentType;
	_contentLength		= other._contentLength;
	_content			= other._content;
	_HTTPResponse		= other._HTTPResponse;
	return (*this);
}

///////////////////////////////////////////////////////////////////
///                    SETTERS				                     //
///////////////////////////////////////////////////////////////////

void	Response::setStatusNum(int number)
{
	this->_statusNum = number;
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
	_content = content;
}

void	Response::setContent(std::vector<char> content)
{
	_content = std::string(content.begin(), content.end());
}

void	Response::setRequest(Request* request)
{
	_request = request;
}

void	Response::setRoutedUrl(std::string url)
{
	std::cout << "url is " << url << " \n";
	std::cout << "default file is " << _request->getRoute()->getDefaultFiles()[0] << " \n";
	
	if (url[url.size() - 1] == '/') // url == "/" ??
		_routedPath = _request->getRoute()->getRootDirectory() + "/" + _request->getRoute()->getDefaultFiles()[0];
	else
		_routedPath = _request->getRoute()->getRootDirectory() + url;


	std::cout << GREEN << _routedPath << STOP_COLOR;
}

void Response::AddHTTPHeaders()
{
	Status status(this->_statusNum);
	if (status.getStatusCode() >= 400) // if its an error
		this->_content = createErrorPageContent(status);
	this->_contentLength = this->_content.size();

	std::stringstream	header;
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

int			Response::getStatus() const
{
	return (_statusNum);
}

///////////////////////////////////////////////////////////////////
///                     MEMBER FUNCTIONS                         //
///////////////////////////////////////////////////////////////////

std::string Response::createErrorPageContent(const Status &num)
{
	std::ifstream inputErrorFile;
	std::string errorFile = _request->getConf().getRoot() + "/" + "error.html";
	inputErrorFile.open(errorFile.c_str(), std::ifstream::in);
	std::stringstream outputString;
	std::string line;

	if (!inputErrorFile.is_open())
	{
		std::cerr << RED << "Could not open error file: " << errorFile << STOP_COLOR << std::endl;
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
