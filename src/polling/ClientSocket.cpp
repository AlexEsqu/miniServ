#include "ClientSocket.hpp"
#include "server.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ClientSocket::ClientSocket(Sockette &Source)
{
	#ifdef DEBUG
		std::cout << "ClientSocket Constructor called" << std::endl;
	#endif

	int addrlen = sizeof(Source.getSocketAddr());

	int socketFd = accept(Source.getSocketFd(), \
		(struct sockaddr *)Source.getSocketAddr(), \
		(socklen_t *)&addrlen);

	setSocketFd(socketFd);

	if (socketFd < 0)
		throw failedSocketAccept();

	memset(_buffer, '\0', sizeof _buffer);
}

//--------------------------- DESTRUCTORS -----------------------------------//


//------------------------------ GETTER --------------------------------------//

char	*ClientSocket::getRequest()
{
	return (_buffer);
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


