#include "HTTPError.hpp"

HTTPError::HTTPError(Request& req, int status)
{
	Response res(req, status);

	_message = res.getHTTPResponse();
}

std::string	HTTPError::getErrorPage()
{
	return _message;
}

const char*		HTTPError::timeout::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request Timeout");
}

const char*		HTTPError::badSyntax::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request has bad syntax");
}

const char*		HTTPError::missingLength::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request has no content length");
}

const char*		HTTPError::contentTooLarge::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request content is too big");
}

const char*		HTTPError::forbiddenMethod::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request has a forbidden method");
}

const char*		HTTPError::badProtocol::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request uses unsupported protocol");
}
