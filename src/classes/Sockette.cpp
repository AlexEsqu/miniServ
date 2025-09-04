#include "Sockette.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Sockette::Sockette()
	: _socketFd(-1)
	, _socketAddrLen(sizeof(_socketAddress))
	,  _port(8080)
{
	#ifdef DEBUG
		std::cout << "Sockette generic Constructor called" << std::endl;
	#endif
}

//--------------------------- DESTRUCTORS -----------------------------------//

Sockette::~Sockette()
{
	#ifdef DEBUG
		std::cout << "Sockette Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

Sockette		&Sockette::operator=(const Sockette &other)
{
	if (this == &other)
		return *this;

	_socketFd = other._socketFd;
	_socketAddress = other._socketAddress;
	_socketAddrLen = other._socketAddrLen;
	_port = other._port;
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

int					Sockette::getSocketFd() const
{
	return _socketFd;
}

sockaddr_in*		Sockette::getSocketAddr()
{
	return &_socketAddress;
}

const sockaddr_in*	Sockette::getSocketAddr() const
{
	return &_socketAddress;
}

int					Sockette::getSocketAddrLen() const
{
	return _socketAddrLen;
}

int				Sockette::getPort() const
{
	return _port;
}

//---------------------------- SETTERS --------------------------------------//

void			Sockette::setPort(int port)
{
	_port = port;
}

void			Sockette::setSocketFd(int socketFd)
{
	_socketFd = socketFd;
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//

void			Sockette::setSocketOption(int option)
{
	const int on = 1;
	if (setsockopt(_socketFd, SOL_SOCKET, option, &on, sizeof(on)) != 0)
		throw failedSocketSetOption();
}

void			Sockette::bindToIPAddress()
{
	int addrlen = sizeof(_socketAddress);

	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_addr.s_addr = INADDR_ANY;
	_socketAddress.sin_port = htons(_port);
	memset(_socketAddress.sin_zero, '\0', sizeof _socketAddress.sin_zero);

	if (bind(_socketFd, (struct sockaddr *)&_socketAddress, (socklen_t)addrlen) < 0)
		throw failedSocketBinding();
}

void			Sockette::setListenMode(int maxQueue)
{
	if (listen(_socketFd, maxQueue) < 0)
		throw failedSocketListen();
}

void			Sockette::acceptConnectionFrom(Sockette ConnectingSocket)
{
	int addrlen = sizeof(ConnectingSocket.getSocketAddr());

	_socketFd = accept(ConnectingSocket.getSocketFd(), \
		(struct sockaddr *)ConnectingSocket.getSocketAddr(), \
		(socklen_t *)&addrlen);

	if (_socketFd < 0)
		throw failedSocketAccept();
}

//--------------------------- EXCEPTIONS ------------------------------------//

const char*		Sockette::failedSocketCreation::what() const throw()
{
	return "ERROR: Failed to create the socket in call to socket()";
}

const char*		Sockette::failedSocketSetOption::what() const throw()
{
	return "ERROR: Failed to set socket option in call to setsockopt()";
}

const char*		Sockette::failedSocketBinding::what() const throw()
{
	return "ERROR: Failed to bind socket in call to bind()";
}

const char*		Sockette::failedSocketListen::what() const throw()
{
	return "ERROR: Failed to listen with socket in call to listen()";
}

const char*		Sockette::failedSocketAccept::what() const throw()
{
	return "ERROR: Failed to accept connection in call to accept()";
}
