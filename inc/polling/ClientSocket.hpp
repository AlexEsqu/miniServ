#pragma once
# include "Sockette.hpp"
#include <string>
#define BUFFSIZE 1000000
#include "server.hpp"
#include <algorithm>

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	char	_buffer[BUFFSIZE];
	std::string	_header;
	std::string _body;
	bool	_isChunked;
	size_t	_contentLength;
public:

	//----------------- CONSTRUCTORS ---------------------//

	ClientSocket(ServerSocket &server);

	//----------------- DESTRUCTOR -----------------------//

	//----------------------- GETTER ---------------------//

	char	*getRequest();

	//----------------- MEMBER FUNCTION ------------------//

	void	readRequest();
	
	void readRequestHeader();
	std::string readRequestBody(std::istringstream &buffer);
};
