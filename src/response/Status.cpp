#include "Status.hpp"

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS | DESTRUCTORS                  //
///////////////////////////////////////////////////////////////////

// create a HTTP status code objects initialized at 200
Status::Status()
	: _statusCode(OK)
	, _hasError(false)
{
	setStatusCode(OK);
}

// create a HTTP status code objects initialized at the given int
Status::Status(e_status num)
	: _statusCode(OK)
	, _hasError(false)
{
	setStatusCode(num);
}

Status::Status(const Status &copy)
{
	this->setStatusCode(copy.getStatusCode());

	*this = copy;
}

Status::~Status()
{
}

///////////////////////////////////////////////////////////////////
///                        OPERATORS                             //
///////////////////////////////////////////////////////////////////

Status&			Status::operator=(const Status &other)
{
	this->_statusCode = other.getStatusCode();
	this->_statusMessage = other.getStatusMessage();
	return (*this);
}

std::ostream&	operator<<(std::ostream &o, Status &status)
{
	o << status.getStatusCode() << " "  << status.getStatusMessage();
	return(o);
}
///////////////////////////////////////////////////////////////////
///                    GETTERS | SETTERS                         //
///////////////////////////////////////////////////////////////////

e_status	Status::getStatusCode() const
{
	return(this->_statusCode);
}

std::string		Status::getStringStatusCode() const
{
	std::stringstream temp;
	temp << this->_statusCode;
	return (temp.str());
}

std::string		Status::getStatusMessage() const
{
	return _statusMessage;
}

bool			Status::hasError() const
{
	return _hasError;
}

void			Status::setError(bool value)
{
	_hasError = value;
}

void			Status::setStatusCode(e_status statusCode)
{
	if (_hasError)
		return;

	_statusCode = statusCode;
	setStatusMessage(_statusCode);
	_hasError = (static_cast<int>(statusCode) >= 400);
	verboseLog("setting status to " + _statusMessage);
}

void			Status::setStatusMessage(e_status code)
{
	static const std::map<e_status, std::string>	status = createStatusLookup();

	std::map<e_status, std::string>::const_iterator res = status.find(code);

	if (res == status.end())
		throw UnknownStatusException();

	_statusMessage = res->second;
}

void			Status::reset()
{
	_hasError = false;
	setStatusCode(OK);
}

std::map<e_status, std::string>	Status::createStatusLookup()
{
	std::cout << "creating a lookup map by hand like a C++98 peasant\n";
	std::map<e_status, std::string> statusLookup;

	statusLookup[CONTINUE] = "Continue";
	statusLookup[SWITCHING_PROTOCOLS] = "Switching Protocols";
	statusLookup[PROCESSING] = "Processing";
	statusLookup[OK] = "OK";
	statusLookup[CREATED] = "Created";
	statusLookup[ACCEPTED] = "Accepted";
	statusLookup[NON_AUTHORITATIVE_INFORMATION] = "Non-Authoritative Information";
	statusLookup[NO_CONTENT] = "No Content";
	statusLookup[RESET_CONTENT] = "Reset Content";
	statusLookup[PARTIAL_CONTENT] = "Partial Content";
	statusLookup[MULTI_STATUS] = "Multi-status";
	statusLookup[ALREADY_REPORTED] = "Already Reported";
	statusLookup[IM_USED] = "IM Used";
	statusLookup[MULTIPLE_CHOICES] = "Multiple Choices";
	statusLookup[MOVED_PERMANENTLY] = "Moved Permanently";
	statusLookup[FOUND] = "Found";
	statusLookup[SEE_OTHER] = "See Other";
	statusLookup[NOT_MODIFIED] = "Not Modified";
	statusLookup[USE_PROXY] = "Use Proxy";
	statusLookup[SWITCH_PROXY] = "Switch Proxy";
	statusLookup[TEMPORARY_REDIRECT] = "Temporary Redirect";
	statusLookup[PERMANENT_REDIRECT] = "Permanent Redirect";
	statusLookup[BAD_REQUEST] = "Bad Request";
	statusLookup[UNAUTHORIZED] = "Unauthorized";
	statusLookup[PAYMENT_REQUIRED] = "Payment Required";
	statusLookup[FORBIDDEN] = "Forbidden";
	statusLookup[NOT_FOUND] = "Not Found";
	statusLookup[METHOD_NOT_ALLOWED] = "Method Not Allowed";
	statusLookup[NOT_ACCEPTABLE] = "Not Acceptable";
	statusLookup[PROXY_AUTHENTICATION_REQUIRED] = "Proxy Authentication Required";
	statusLookup[REQUEST_TIMEOUT] = "Request Timeout";
	statusLookup[CONFLICT] = "Conflict";
	statusLookup[GONE] = "Gone";
	statusLookup[LENGTH_REQUIRED] = "Length Required";
	statusLookup[PRECONDITION_FAILED] = "Precondition Failed";
	statusLookup[PAYLOAD_TOO_LARGE] = "Payload Too Large";
	statusLookup[URI_TOO_LONG] = "URI Too Long";
	statusLookup[UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type";
	statusLookup[RANGE_NOT_SATISFIABLE] = "Range Not Satisfiable";
	statusLookup[EXPECTATION_FAILED] = "Expectation Failed";
	statusLookup[I_AM_A_TEAPOT] = "I'm a teapot";
	statusLookup[MISDIRECTED_REQUEST] = "Misdirected Request";
	statusLookup[UNPROCESSABLE_ENTITY] = "Unprocessable Entity";
	statusLookup[LOCKED] = "Locked";
	statusLookup[FAILED_DEPENDENCY] = "Failed Dependency";
	statusLookup[TOO_EARLY] = "Too Early";
	statusLookup[UPGRADE_REQUIRED] = "Upgrade Required";
	statusLookup[PRECONDITION_REQUIRED] = "Precondition Required";
	statusLookup[TOO_MANY_REQUESTS] = "Too Many Requests";
	statusLookup[REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large";
	statusLookup[UNAVAILABLE_FOR_LEGAL_REASONS] = "Unavailable For Legal Reasons";
	statusLookup[INTERNAL_SERVER_ERROR] = "Internal Server Error";
	statusLookup[NOT_IMPLEMENTED] = "Not Implemented";
	statusLookup[BAD_GATEWAY] = "Bad Gateway";
	statusLookup[SERVICE_UNAVAILABLE] = "Service Unavailable";
	statusLookup[GATEWAY_TIMEOUT] = "Gateway Timeout";
	statusLookup[HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
	statusLookup[VARIANT_ALSO_NEGOTIATES] = "Variant Also Negotiates";
	statusLookup[INSUFFICIENT_STORAGE] = "Insufficient Storage";
	statusLookup[LOOP_DETECTED] = "Loop Detected";
	statusLookup[NOT_EXTENDED] = "Not Extended";
	statusLookup[NETWORK_AUTHENTICATION_REQUIRED] = "Network Authentication Required";

	return statusLookup;
}

///////////////////////////////////////////////////////////////////
///                     MEMBER FUNCTIONS                         //
///////////////////////////////////////////////////////////////////
