#include "ClientSocket.hpp"
#include "server.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ClientSocket::ClientSocket(ServerSocket &server)
	:_serv(server)
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

char*	ClientSocket::getBuffer()
{
	return (_buffer);
}

struct epoll_event&	ClientSocket::getEvent()
{
	return (_event);
}

Request*	ClientSocket::getRequest()
{
	return (_request);
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

void	ClientSocket::readRequest()
{
	// read the Client's request into a buffer
	int valread = read(getSocketFd(), _buffer, 30000);
	if (valread < 0)
		throw failedSocketRead();

	#ifdef DEBUG
		std::cout << "Answer socket read " << valread << " bytes: [" << _buffer << "]\n" << std::endl;
	#endif

	// add buffer content to a Request object
	if (_request == NULL)
		_request = new Request(_serv.getConf(), _buffer);
	else
		_request->addRequestChunk(_buffer);

	// clear buffer for further use
	memset(_buffer, '\0', sizeof _buffer);
}


