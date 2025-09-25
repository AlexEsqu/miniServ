#pragma once
# include "Sockette.hpp"
# include <sys/epoll.h>

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	char				_buffer[30000];
	struct epoll_event	_event;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ClientSocket(ServerSocket &server);

	//----------------- DESTRUCTOR -----------------------//


	//----------------------- SETTER ---------------------//

	void	setEvent(uint32_t epollEventMask);

	//----------------------- GETTER ---------------------//

	char*				getRequest();
	struct epoll_event&	getEvent();

	//----------------- MEMBER FUNCTION ------------------//

	void	readRequest();

};
