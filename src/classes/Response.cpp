#include "Response.hpp"

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS | DESTRUCTORS                  //
///////////////////////////////////////////////////////////////////

Response::Response()
{
	// std::cout << "Response Constructor called" << std::endl;
}

Response::Response(const Response &copy)
{
	// std::cout << "Response copy Constructor called" << std::endl;
	*this = copy;
}

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
	this->_requestedURL = url;
}

void Response::setResponse(std::string response)
{
	this->_response = response;
}

void Response::setProtocol(std::string protocol)
{
	this->_protocol = protocol;
}

///////////////////////////////////////////////////////////////////
///                     MEMBER FUNCTIONS                         //
///////////////////////////////////////////////////////////////////

std::string Response::createResponse()
{
	std::stringstream response;
	Status statusCode(this->_statusNum);
	response << this->_protocol << " " << statusCode 
			<< "Content-Type: " << this->_contentType << "\n"
			<< "Content-Length: " << this->_contentLength
			<< "\n\n" << this->_content;
	std::cout << response.rdbuf();

	return(response.str());
}