#include "Status.hpp"

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS | DESTRUCTORS                  //
///////////////////////////////////////////////////////////////////

// create a HTTP status code objects initialized at 200
Status::Status()
{
	setStatusCode(OK);
}

// create a HTTP status code objects initialized at the given int
Status::Status(e_status num)
{
	Status::setStatusCode(num);
}

Status::Status(const Status &copy)
{
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
	return(temp.str());
}

std::string		Status::getStatusMessage() const
{
	return(this->_statusMessage);
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

	// verboseLog("setting status to " + statusCode);
	this->_statusCode = statusCode;
	try
	{
		if (statusCode <= 511 && *_statusMessages[statusCode] != '\0') //add macro for readability
			this->_statusMessage = _statusMessages[statusCode];
		else
			throw Status::UnknownStatusException();
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << e.what() << STOP_COLOR << '\n';
	}

	if (statusCode >= BAD_REQUEST)
		_hasError = true;
}

void			Status::setStatusMessage(std::string message)
{
	this->_statusMessage = message;
	int i = 0;
	try
	{
		while (i < 512)
		{
			if (_statusMessages[i] == message)
			{
				this->_statusCode = static_cast<e_status>(i);
				return;
			}
			i++;
		}
		if (i == 512)
			throw Status::UnknownStatusException();
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << e.what() << STOP_COLOR << '\n';
	}
}

///////////////////////////////////////////////////////////////////
///                     MEMBER FUNCTIONS                         //
///////////////////////////////////////////////////////////////////
