#include "ServerAnswer.hpp"
#include "server.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerAnswer::ServerAnswer(Sockette &Source)
{
	#ifdef DEBUG
		std::cout << "ServerAnswer Constructor called" << std::endl;
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

char	*ServerAnswer::getRequest()
{
	return (_buffer);
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

void	ServerAnswer::readRequest()
{
	int valread = read(getSocketFd(), _buffer, 30000);

	if (valread < 0)
		throw failedSocketRead();

	// #ifdef DEBUG
	// 	std::cout << "Answer socket read " << valread << " bytes: [" << _buffer << "]\n" << std::endl;
	// #endif
}


