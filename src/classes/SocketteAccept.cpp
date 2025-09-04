#include "SocketteAccept.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

SocketteAccept::SocketteAccept(Sockette &Source)
{
	#ifdef DEBUG
		std::cout << "SocketteAccept Constructor called" << std::endl;
	#endif

	int addrlen = sizeof(Source.getSocketAddr());

	int socketFd = accept(Source.getSocketFd(), \
		(struct sockaddr *)Source.getSocketAddr(), \
		(socklen_t *)&addrlen);

	if (socketFd < 0)
		throw failedSocketAccept();
	setSocketFd(socketFd);

}

//--------------------------- DESTRUCTORS -----------------------------------//

SocketteAccept::~SocketteAccept()
{
	#ifdef DEBUG
		std::cout << "SocketteAccept Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

