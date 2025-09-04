#include "Sockette.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Sockette::Sockette()
	: _socketFd(-1)
{
	#ifdef DEBUG
		std::cout << "Sockette Constructor called" << std::endl;
	#endif

	// Creating socket and file descriptor referring it
	if ((_socketFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		throw failedSocketCreation();

	// allow socket to be reused and webserv to reload faster with SO_REUSEADDR
	setSocketOption(SO_REUSEADDR);

}

Sockette::Sockette(const Sockette &copy)
{
	#ifdef DEBUG
		std::cout << "Sockette copy Constructor called" << std::endl;
	#endif
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

Sockette::~Sockette()
{
	#ifdef DEBUG
		std::cout << "Sockette Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

Sockette	&Sockette::operator=(const Sockette &other)
{
	if (this == &other)
		return *this;

	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

int			Sockette::getSocketFd()
{
	return (_socketFd);
}

sockaddr_in	Sockette::getSocketAddr()
{
	return (_socketAddress);
}

int			Sockette::getSocketAddrLen()
{
	return (_socketAddrLen);
}

//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//

void		Sockette::setSocketOption(int option)
{
	const int on = 1;
	if (setsockopt(_socketFd, SOL_SOCKET, option, &on, sizeof(on)) != 0)
		throw failedSocketSetOption();
}


//--------------------------- EXCEPTIONS ------------------------------------//

const char*	Sockette::failedSocketCreation::what() const throw()
{
	return "ERROR: Failed to create the socket in call to socket()";
}

const char*	Sockette::failedSocketSetOption::what() const throw()
{
	return "ERROR: Failed to set socket option in call to setsockopt()";
}
