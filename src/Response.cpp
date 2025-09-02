#include "Response.hpp"

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS | DESTRUCTORS                  //
///////////////////////////////////////////////////////////////////

Response::Response()
{
	std::cout << "Response Constructor called" << std::endl;
}

Response::Response(const Response &copy)
{
	std::cout << "Response copy Constructor called" << std::endl;
	*this = copy;
}

Response::~Response()
{
	std::cout << "Response Destructor called" << std::endl;
}

///////////////////////////////////////////////////////////////////
///                        OPERATORS                             //
///////////////////////////////////////////////////////////////////

Response &Response::operator=(const Response &other)
{
	// code
	return (*this);
}

///////////////////////////////////////////////////////////////////
///                    GETTERS | SETTERS                         //
///////////////////////////////////////////////////////////////////

void Response::setStatusNum(int number)
{
	this->_statusNum = number;
}

void Response::setMethod(int method)
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
	this->_content = content;
}

void Response::setUrl(std::string url)
{
	this->_url = url;
}

void Response::setResponse(std::string response)
{
	this->_response = response;
}

///////////////////////////////////////////////////////////////////
///                     MEMBER FUNCTIONS                         //
///////////////////////////////////////////////////////////////////
