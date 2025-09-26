#pragma once

#include <sys/epoll.h>
#include <string>
#include <algorithm>

#include "server.hpp"
#include "Sockette.hpp"

#define BUFFSIZE 1000000

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	ServerSocket&		_serv;
	char				_buffer[BUFFSIZE];
	struct epoll_event	_event;
	Request*			_request;
	std::string	_header;
	std::string _body;
	bool	_isChunked;
	size_t	_contentLength;

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
	void 				readRequestHeader();
	std::string 		readRequestBody(std::istringstream &buffer);
};
