#include "Status.hpp"

///////////////////////////////////////////////////////////////////
///                  CONSTRUCTORS | DESTRUCTORS                  //
///////////////////////////////////////////////////////////////////

Status::Status()
{
	std::cout << "Status Constructor called" << std::endl;
}

Status::Status(int num) : _statusCode(num)
{
	try
	{
		if (num <= 511 && _statusMessages[num] != NULL)
			this->_statusMessage = _statusMessages[num];
		else
			throw Status::UnknownStatusException();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
}

Status::Status(std::string message) : _statusMessage(message)
{
	int i = 0;
	try
	{
		while (i < 512)
		{
			if (_statusMessages[i] == message)
			{
				this->_statusCode = i;
			}
			i++;
		}
		if (i == 512)
			throw Status::UnknownStatusException();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
}

Status::Status(const Status &copy)
{
	std::cout << "Status copy Constructor called" << std::endl;
	*this = copy;
}

Status::~Status()
{
	std::cout << "Status Destructor called" << std::endl;
}

///////////////////////////////////////////////////////////////////
///                        OPERATORS                             //
///////////////////////////////////////////////////////////////////

Status &Status::operator=(const Status &other)
{
	// code
	return (*this);
}

///////////////////////////////////////////////////////////////////
///                    GETTERS | SETTERS                         //
///////////////////////////////////////////////////////////////////

unsigned int Status::getStatusCode()
{
	return(this->_statusCode);
}

std::string Status::getStatusMessage()
{
	return(this->_statusMessage);
}

///////////////////////////////////////////////////////////////////
///                     MEMBER FUNCTIONS                         //
///////////////////////////////////////////////////////////////////
