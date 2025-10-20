#pragma once

#include <sys/epoll.h>
#include <string>
#include <algorithm>

#include "Sockette.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define BUFFSIZE 64000
#define END_OF_HEADER_STR "\r\n\r\n"

enum e_clientState {
	CLIENT_CONNECTED,	// Initial connection established
	CLIENT_PARSING,		// Currently parsing incoming HTTP request
	CLIENT_HAS_PARSED,	// Request parsing completed
	CLIENT_FILLING,		// Currently generating response content
	CLIENT_HAS_FILLED,	// Response content ready
	CLIENT_SENDING,		// Currently sending response to client
	CLIENT_HAS_SENT		// Response fully sent
};

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	ServerSocket&		_serv;
	char				_buffer[BUFFSIZE];

	Status				_status;
	Request*			_request;
	Response*			_response;

	bool				_isReadingFromPipe;
	int					_readingEndOfCgiPipe;

	e_clientState		_clientState;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ClientSocket(ServerSocket &server);

	//----------------- DESTRUCTOR -----------------------//

	~ClientSocket();

	//----------------------- SETTER ---------------------//

	void				setClientState(e_clientState state);

	//----------------------- GETTER ---------------------//

	char*				getBuffer();
	Request*			getRequest();
	ServerSocket&		getServer();
	Response*			getResponse();
	int					getCgiPipeFd();

	e_clientState		getClientState() const;

	bool				isReadingFromPipe() const;

	//----------------- MEMBER FUNCTION ------------------//

	void				checkForReadError(int valread);

	void				readRequest();
	void				createNewResponse();
	void				startReadingPipe(int pipeFd);
	void				stopReadingPipe();
	void				sendResponse();
	void				deleteResponse();

	bool				hasRequest() const;
	bool				hasParsedRequest();
	bool				hasFilledResponse() const;
	bool				hasSentResponse() const;
	void				resetRequest();

};
