#include "Response.hpp"
#include <fcntl.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS                                //
///////////////////////////////////////////////////////////////////

Response::Response(Request *req)
	: _statusNum(200)
	, _requestedFileName(req->getRequestedURL())
	, _request(req)
{
	setMethod(_request->getMethod());
	if (this->_method == "POST")
		setStatusNum(201);
	if (this->_method == "GET")
		setStatusNum(200);
	setUrl(_requestedFileName);
}

Response::Response(Request *req, int status)
	: _statusNum(status)
	, _requestedFileName(req->getRequestedURL())
	, _request(req)
{
	if (status >= 400)
		setHTTPResponse();
}

Response::Response(const Response &copy)
	: _request(copy._request)
{
	// std::cout << "Response copy Constructor called" << std::endl;
	*this = copy;
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
	// code
	(void)other;
	return (*this);
}

///////////////////////////////////////////////////////////////////
///                    SETTERS				                     //
///////////////////////////////////////////////////////////////////

void Response::setStatusNum(int number)
{
	this->_statusNum = number;
}

void Response::setMethod(std::string method)
{
	this->_method = method;
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
	_content = content;
}

void Response::setContent(std::vector<char> content)
{
	_content = std::string(content.begin(), content.end());
}

void Response::setUrl(std::string url)
{
	std::string root = _request->getConf().getRootMatchForRequestedFile(url)->getRootDirectory();
	if (url == "/")
		this->_requestedFileName = root + _request->getConf().getRoutes(0)->getDefaultFiles()[0];
	else
		this->_requestedFileName = root + url;

	// GET /admin/truc => GET /www/var/etc/admin/
	// GET /bidule/chose => GET /start/truc
	// GET / => GET <root>/index.html ou <root>/index.php
	// std::cout << GREEN << _requestedFileName << STOP_COLOR;
}

void Response::setHTTPResponse()
{
	Status status(this->_statusNum);
	if (status.getStatusCode() >= 400) // if its an error
		this->_content = createErrorPageContent(status);
	this->_contentLength = this->_content.size();

	std::stringstream	header;
	header << _request->getProtocol() << " " << status;
	if (this->_method == "GET")
	{
		header << "Content-Type: " << _contentType << "\r\n"
				 << "Content-Length: " << _contentLength << "\r\n"
				 << "\r\n";
	}
	if (this->_method == "POST")
	{
		header << "Content-Type: text/html\r\n"
				<< "Content-Length: 0\r\n"
				<< "Refresh: 0; url=/\r\n"
				<< "\r\n";
	}

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
	return (_requestedFileName);
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
	std::string errorFile = _request->getConf().getRoutes(0)->getRootDirectory() + "error.html";
	inputErrorFile.open(errorFile.c_str(), std::ifstream::in);
	std::stringstream outputString;
	std::string line;

	if (!inputErrorFile.is_open())
	{
		std::cerr << RED << "Could not open error file: " << errorFile << STOP_COLOR << std::endl;
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
