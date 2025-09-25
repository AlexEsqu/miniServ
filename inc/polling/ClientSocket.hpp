#pragma once
# include "Sockette.hpp"

class ServerSocket;

class ClientSocket: public Sockette
{

private:

	char	_buffer[30000];

public:

	//----------------- CONSTRUCTORS ---------------------//

	ClientSocket(ServerSocket &server);

	//----------------- DESTRUCTOR -----------------------//

	//----------------------- GETTER ---------------------//

	char	*getRequest();

	//----------------- MEMBER FUNCTION ------------------//

	void	readRequest();

};
