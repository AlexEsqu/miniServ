#pragma once

#include <sys/epoll.h>
#include <string>
#include <algorithm>

#include "Sockette.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define BUFFSIZE 64000
#define MAX_HEADER_SIZE 4000
#define END_OF_HEADER_STR "\r\n\r\n"

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	ServerSocket&		_serv;
	char				_buffer[MAX_HEADER_SIZE];
	std::string			_fullHeader;
	Request*			_request;
	std::string			_response;

	size_t				_headerSize;

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

	void				checkForReadError(int valread);
	bool				tryToReadHeaderBlock();
	bool				tryToReadBodyBlock();
	bool 				tryToReadChunkBodyBlock();
	void				readRequest();
	void				sendResponse();
};
