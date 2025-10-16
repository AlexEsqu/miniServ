#include "Status.hpp"

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS | DESTRUCTORS                  //
///////////////////////////////////////////////////////////////////

// create a HTTP status code objects initialized at 200
Status::Status()
{
	setStatusCode(200);
}

// create a HTTP status code objects initialized at the given int
Status::Status(unsigned int num)
{
	Status::setStatusCode(num);
}

// TO DO : Elvire, this feels dangerous, can we delete this function ?
Status::Status(std::string message)
{
	Status::setStatusMessage(message);
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

unsigned int	Status::getStatusCode() const
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

void			Status::setStatusCode(unsigned int num)
{
	this->_statusCode = num;
	try
	{
		if (num <= 511 && *_statusMessages[num] != '\0') //add macro for readability
			this->_statusMessage = _statusMessages[num];
		else
			throw Status::UnknownStatusException();
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << e.what() << STOP_COLOR << '\n';
	}

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
				this->_statusCode = i;
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
