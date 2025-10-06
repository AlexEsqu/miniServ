#pragma once

#include <sys/epoll.h>
#include <string>
#include <algorithm>

#include "Sockette.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define BUFFSIZE 1000000

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	ServerSocket&		_serv;
	char				_buffer[BUFFSIZE];
	Request*			_request;
	std::string			_response;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ClientSocket(ServerSocket &server);

	//----------------- DESTRUCTOR -----------------------//

	~ClientSocket();

	//----------------------- SETTER ---------------------//

	void				setResponse(std::string response);
	void				resetRequest();

	//----------------------- GETTER ---------------------//

	char*				getBuffer();
	Request*			getRequest();
	ServerSocket&		getServer();
	std::string&		getResponse();

	//----------------- MEMBER FUNCTION ------------------//

	void				readRequest();
	void				sendResponse();
};
