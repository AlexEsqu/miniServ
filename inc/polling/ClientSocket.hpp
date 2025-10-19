#pragma once

#include <sys/epoll.h>
#include <string>
#include <algorithm>

#include "Sockette.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define BUFFSIZE 64000
#define END_OF_HEADER_STR "\r\n\r\n"

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	ServerSocket&		_serv;
	char				_buffer[BUFFSIZE];

	Request*			_request;
	Response*			_response;

	bool				_isReadingFromPipe;
	int					_readingEndOfCgiPipe;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ClientSocket(ServerSocket &server);

	//----------------- DESTRUCTOR -----------------------//

	~ClientSocket();

	//----------------------- SETTER ---------------------//

	// void				setResponse(std::string response);
	void				resetRequest();

	//----------------------- GETTER ---------------------//

	char*				getBuffer();
	Request*			getRequest();
	ServerSocket&		getServer();
	Response*			getResponse();
	int					getCgiPipeFd();

	bool				hasRequest() const;
	bool				hasParsedRequest() const;
	bool				hasFilledResponse() const;
	bool				hasSentFullResponse() const;

	bool				isReadingFromPipe() const;

	//----------------- MEMBER FUNCTION ------------------//

	void				checkForReadError(int valread);
	bool				tryToReadHeaderBlock();
	bool				tryToReadBodyBlock();
	bool 				tryToReadChunkBodyBlock();

	void				readRequest();
	void				createNewResponse();
	void				startReadingPipe(int pipeFd);
	void				stopReadingPipe();
	void				sendResponse();
	void				deleteResponse();

};
