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

const char*		timeout::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request Timeout");
}

const char*		badSyntax::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request has bad syntax");
}

const char*		missingLength::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request has no content length");
}

const char*		contentTooLarge::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request content is too big");
}

const char*		forbiddenMethod::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request has a forbidden method");
}

const char*		badProtocol::what() const throw()
{
	return ERROR_FORMAT("ERROR: Request uses unsupported protocol");
}
