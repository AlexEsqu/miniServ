#pragma once

#include <sys/epoll.h>
#include <string>
#include <algorithm>

#include "Sockette.hpp"
#include "Request.hpp"

#define BUFFSIZE 1000000

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	ServerSocket&		_serv;
	char				_buffer[BUFFSIZE];
	struct epoll_event	_event;
	Request*			_request;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ClientSocket(ServerSocket &server);

	//----------------- DESTRUCTOR -----------------------//

	~ClientSocket();

	//----------------------- SETTER ---------------------//

	void				setEvent(uint32_t epollEventMask);
	void				resetRequest();

	//----------------------- GETTER ---------------------//

	char*				getBuffer();
	struct epoll_event&	getEvent();
	Request*			getRequest();
	ServerSocket&		getServer();

	//----------------- MEMBER FUNCTION ------------------//

	void				readRequest();
};
