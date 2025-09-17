#include "Response.hpp"
#include <fcntl.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS | DESTRUCTORS                  //
///////////////////////////////////////////////////////////////////

Response::Response()
{
	// std::cout << "Response Constructor called" << std::endl;
}

Response::Response(Request &req, int status) : _content("")
{

	this->_statusNum = status;
	this->_contentType = req.getContentType();
	this->_protocol = req.getProtocol();
	Response::setUrl(req.getRequestedURL());
	if (status >= 400 )
	{
		setHTTPResponse();
		return;
	}
	Response::setContent(this->_content);
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

int is_directory(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

void Response::setContent(std::string content)
{
	if (content.empty())
	{
		std::ifstream input(this->_requestedFileName.c_str()); // opening the file as the content for the response
		std::stringstream content;

		if (!input.is_open() || is_directory(this->_requestedFileName.c_str()))
		{
			std::cerr << RED << "Could not open file" << STOP_COLOR << std::endl;
			Response::setStatusNum(404);
			std::cout << _statusNum << std::endl;

			Response::setHTTPResponse();
			return;
		}
		content << input.rdbuf();
		this->_content = content.str();
	}
	else
	{
		this->_content = content;
	}
	Response::setHTTPResponse();
}

void Response::setUrl(std::string url)
{
	if (url == "./")
		this->_requestedFileName = conf.getRoutes(0)->getRootDirectory() + conf.getRoutes(0)->getDefaultFiles()[0];
	else
		this->_requestedFileName = conf.getRoutes(0)->getRootDirectory() + url;
	std::cout << GREEN << _requestedFileName << STOP_COLOR << std::endl;
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

std::string Response::createErrorPageContent(const Status &num)
{
	std::ifstream inputErrorFile;
	std::string errorFile = conf.getRoutes(0)->getRootDirectory() + "error.html";
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

void Response::setHTTPResponse()
{
	std::stringstream response;
	Status status(this->_statusNum);
	if (status.getStatusCode() >= 400) // if its an error
	{
		this->_content = createErrorPageContent(status);
	}
	this->_contentLength = this->_content.length();
	response << this->_protocol << " " << status
			 << "Content-Type: " << this->_contentType << "\n"
			 << "Content-Length: " << this->_contentLength
			 << "\n\n"
			 << this->_content;
	std::cout << response.rdbuf();

	this->_HTTPResponse = response.str();
}

std::string Response::getHTTPResponse() const
{
	return(this->_HTTPResponse);
}
