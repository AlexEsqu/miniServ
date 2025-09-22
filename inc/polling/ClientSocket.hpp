#pragma once
# include "Sockette.hpp"

class ClientSocket: public Sockette
{

private:

	char	_buffer[30000];

public:

	//----------------- CONSTRUCTORS ---------------------//

	ClientSocket(Sockette &ListenSocket);

	//----------------- DESTRUCTOR -----------------------//

	//----------------------- GETTER ---------------------//

	char	*getRequest();

	//----------------- MEMBER FUNCTION ------------------//

	void	readRequest();

};
