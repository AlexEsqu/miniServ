#pragma once
# include "Sockette.hpp"
# include <sys/epoll.h>

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	ServerSocket&		_serv;
	char				_buffer[30000];
	struct epoll_event	_event;
	Request*			_request;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ClientSocket(ServerSocket &server);

	//----------------- DESTRUCTOR -----------------------//


	//----------------------- SETTER ---------------------//

	void				setEvent(uint32_t epollEventMask);

	//----------------------- GETTER ---------------------//

	char*				getBuffer();
	struct epoll_event&	getEvent();
	Request*			getRequest();

	//----------------- MEMBER FUNCTION ------------------//

	void				readRequest();

};
