#include "SocketteAnswer.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

SocketteAnswer::SocketteAnswer(Sockette &Source)
{
	#ifdef DEBUG
		std::cout << "SocketteAnswer Constructor called" << std::endl;
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

char	*SocketteAnswer::getRequest()
{
	return (_buffer);
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

void	SocketteAnswer::readRequest()
{
	int valread = read(getSocketFd(), _buffer, 30000);

	if (valread < 0)
		throw failedSocketRead();

	#ifdef DEBUG
		std::cout << "Answer socket read :[" <<  _buffer << "]\n" << std::endl;
	#endif
}


