#include "ClientSocket.hpp"
#include "server.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ClientSocket::ClientSocket(ServerSocket &server)
{
	#ifdef DEBUG
		std::cout << "ClientSocket Constructor called" << std::endl;
	#endif

	int addrlen = sizeof(server.getSocketAddr());

	int socketFd = accept(server.getSocketFd(), \
		(struct sockaddr *)server.getSocketAddr(), \
		(socklen_t *)&addrlen);

	if (socketFd < 0) {
		perror("accept() failed with error");
		throw failedSocketAccept();
	}

	setSocketFd(socketFd);

	memset(_buffer, '\0', sizeof _buffer);
}

//--------------------------- DESTRUCTORS -----------------------------------//


//------------------------------ SETTER --------------------------------------//

void	ClientSocket::setEvent(uint32_t epollEventMask)
{
	_event.events = epollEventMask;
	_event.data.fd = getSocketFd();

	// adding new socket pointer as context in the event itself
	_event.data.ptr = this;
}

//------------------------------ GETTER --------------------------------------//

char*	ClientSocket::getRequest()
{
	return (_buffer);
}

struct epoll_event&	ClientSocket::getEvent()
{
	return (_event);
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

void	ClientSocket::readRequest()
{
	int valread = read(getSocketFd(), _buffer, 30000);

	if (valread < 0)
		throw failedSocketRead();

	// #ifdef DEBUG
	// 	std::cout << "Answer socket read " << valread << " bytes: [" << _buffer << "]\n" << std::endl;
	// #endif
}


