#include "Sockette.hpp"
#include "ClientSocket.hpp"


//--------------------------- CONSTRUCTORS ----------------------------------//

Sockette::Sockette()
	: _socketFd(-1)
	, _socketAddrLen(sizeof(_socketAddress))
	,  _port(8080)
{
}

Sockette::Sockette(const Sockette &other)
{
	*this = other;
}

//--------------------------- DESTRUCTORS -----------------------------------//

Sockette::~Sockette()
{
	if (_socketFd >= 0)
		close(_socketFd);
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

int					Sockette::getPort() const
{
	return _port;
}

struct epoll_event&	Sockette::getEpollEventsMask()
{
	return (_epollEvent);
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

void			Sockette::setSocketOption(int option)
{
	const int on = 1;
	if (setsockopt(_socketFd, SOL_SOCKET, option, &on, sizeof(on)) != 0)
		throw failedSocketSetOption();
}

void			Sockette::setSocketNonBlocking()
{
	int status = fcntl(getSocketFd(), F_SETFL, fcntl(getSocketFd(), F_GETFL, 0) | O_NONBLOCK);

	if (status == -1)
		throw failedFcntl();
}

void			Sockette::setListenMode(int maxQueue)
{
	if (listen(_socketFd, maxQueue) < 0) {
		perror("listen() failed");
		throw failedSocketListen();
	}
}

void			Sockette::setEpollEventsMask(uint32_t epollEventMask)
{
	_epollEvent.events = epollEventMask;
	// adding new socket pointer as context in the event itself
	_epollEvent.data.ptr = this;
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//

void			Sockette::bindToIPAddress()
{
	int addrlen = sizeof(_socketAddress);

	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_addr.s_addr = INADDR_ANY;
	_socketAddress.sin_port = htons(_port);
	memset(_socketAddress.sin_zero, '\0', sizeof _socketAddress.sin_zero);

	if (bind(_socketFd, (struct sockaddr *)&_socketAddress, (socklen_t)addrlen) < 0)
	{
		perror("bind() failed");
		throw failedSocketBinding();
	}
}

void			Sockette::acceptConnectionFrom(ClientSocket* ConnectingSocket)
{
	int addrlen = sizeof(ConnectingSocket->getSocketAddr());

	_socketFd = accept(ConnectingSocket->getSocketFd(), \
		(struct sockaddr *)ConnectingSocket->getSocketAddr(), \
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

const char*		Sockette::failedSocketRead::what() const throw()
{
	return "ERROR: Failed to read request in buffer call to read()";
}

const char*		Sockette::failedFcntl::what() const throw()
{
	return "ERROR: Failed to modify socket flags in call to fcntl()";
}

const char*		Sockette::endSocket::what() const throw()
{
	return "LOG: Socket read() returned 0, closing socket";
}
