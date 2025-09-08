#include "Status.hpp"

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS | DESTRUCTORS                  //
///////////////////////////////////////////////////////////////////

Status::Status()
{
	// std::cout << "Status Constructor called" << std::endl;
}

Status::Status(int num) :_statusMessage("Unknown status code"), _statusCode(num)
{
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

Status::Status(std::string message) : _statusMessage(message), _statusCode(0)
{
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

Status::Status(const Status &copy)
{
	// std::cout << "Status copy Constructor called" << std::endl;
	*this = copy;
}

Status::~Status()
{
	// std::cout << "Status Destructor called" << std::endl;
}

///////////////////////////////////////////////////////////////////
///                        OPERATORS                             //
///////////////////////////////////////////////////////////////////

Status &Status::operator=(const Status &other)
{
	this->_statusCode = other.getStatusCode();
	this->_statusMessage = other.getStatusMessage();
	return (*this);
}

std::ostream &operator<<(std::ostream &o, Status &status)
{
	o << status.getStatusCode() << " "  << status.getStatusMessage() << std::endl;
	return(o);
}
///////////////////////////////////////////////////////////////////
///                    GETTERS | SETTERS                         //
///////////////////////////////////////////////////////////////////

unsigned int Status::getStatusCode() const
{
	return(this->_statusCode);
}

std::string Status::getStringStatusCode() const
{
	std::stringstream temp;
	temp << this->_statusCode;
	return(temp.str());
}

std::string Status::getStatusMessage() const
{
	return(this->_statusMessage);
}

///////////////////////////////////////////////////////////////////
///                     MEMBER FUNCTIONS                         //
///////////////////////////////////////////////////////////////////
