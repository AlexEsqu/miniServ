#include "Response.hpp"
#include <fcntl.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS                                //
///////////////////////////////////////////////////////////////////

Response::Response(Request *req)
	: _request(req)
	, _status(req->getStatus())
{
	setRoutedUrl(_request->getRequestedURL());
}

Response::Response(const Response &copy)
	: _request(copy._request)
	, _status(copy._status)
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

	_request			= other._request;
	_status				= other._status;
	_routedPath			= other._routedPath;
	_contentType		= other._contentType;
	_contentLength		= other._contentLength;
	_responsePage		= other._responsePage;
	return (*this);
}

///////////////////////////////////////////////////////////////////
///                    SETTERS				                     //
///////////////////////////////////////////////////////////////////

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

// sets status code but does not raise error flag in request
void Response::setStatus(e_status status)
{
	_request->setStatus(status);
}

// sets status code and raises error flag in request
void	Response::setError(e_status status)
{
	_request->setError(status);
}

bool	isDirectory(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

bool	isValidPath(std::string& path)
{
	std::ifstream in(path.c_str(), std::ios::binary);
	if (in.is_open() && !isDirectory(path.c_str()))
		return true;
	return false;
}

void	Response::routeToDefaultFiles(std::string& url, const Route* route, std::string& root)
{
	for (size_t i = 0; i < route->getDefaultFiles().size(); i++)
	{
		std::string possiblePath = root;
		if (!possiblePath.empty() && *--possiblePath.end() == '/'
			&& !url.empty() && *url.begin() == '/')
			possiblePath.erase(possiblePath.size() - 1);

		possiblePath += url;
		possiblePath += route->getDefaultFiles()[i];
		if (isValidPath(possiblePath))
		{
			_routedPath = possiblePath;
			return;
		}
	}
	_routedPath.clear();
	return;
}

void	Response::routeToFilePath(std::string& url, const Route* route, std::string& root)
{
	std::string possiblePath = root;
	if (!possiblePath.empty() && *--possiblePath.end() == '/'
		&& !url.empty() && *url.begin() == '/')
		possiblePath.erase(possiblePath.size() - 1);

	possiblePath += url;
	if (isValidPath(possiblePath))
	{
		_routedPath = possiblePath;
		return;
	}

	// if it does not find the file path, assume it is a directory missing its ending /
	if (*--url.end() != '/')
		url += '/';
	routeToDefaultFiles(url, route, root);
}

void	Response::routeUrlForPostDel(std::string url)
{
	// std::cout << "size of Routes : " << _request->getConf().getRoutes().size() << "\n";

	_routedPath = url;
}

void	Response::routeUrlForGet(std::string url)
{
	const Route*	route = _request->getRoute();
	if (route == NULL)
	{
		setError(NOT_FOUND);
		return;
	}

	std::string		root = route->getRootDirectory();

	if (!url.empty() && *--url.end() == '/')
		routeToDefaultFiles(url, route, root);

	else
		routeToFilePath(url, route, root);
}

void Response::setRoutedUrl(std::string url)
{
	if (_request->getMethodCode() == GET or _request->getMethodCode() == HEAD)
		routeUrlForGet(url);
	else
		routeUrlForPostDel(url);
}

///////////////////////////////////////////////////////////////////
///                    GETTERS 			                         //
///////////////////////////////////////////////////////////////////

Request		*Response::getRequest()
{
	return (_request);
}

std::string	Response::getRoutedURL() const
{
	return (_routedPath);
}

Status&		Response::getStatus()
{
	return (_request->getStatus());
}

std::string	Response::getHTTPHeaders() const
{
	return (_HTTPHeaders);
}

std::string	Response::getHTTPResponse()
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
	if (getStatus().getStatusCode() >= 400) // if its an error
		setContent(fetchErrorPageContent(getStatus()));

	this->_contentLength = _responsePage.getBufferSize();

	std::stringstream header;
	header << _request->getProtocol() << " " << getStatus() << "\r\n"
		   << "Content-Type: " << _contentType << "\r\n"
		   << "Content-Length: " << _contentLength << "\r\n"
		   << "Connection: " << (_request->isKeepAlive() ? "keep-alive" : "close") << "\r\n"
		   << "Server: miniServ\r\n";

	std::cout << "Content-Length is " << _contentLength << " + " << header.str().size() << "\n";

	if (_request->getMethodAsString() == "POST")
	{
		header << "Refresh: 0; url=/\r\n";
	}

	header << "\r\n";

	_HTTPHeaders = header.str();
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


