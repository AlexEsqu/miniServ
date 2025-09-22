#pragma once
# include "Sockette.hpp"

class ServerAnswer: public Sockette
{

private:

	char	_buffer[30000];

public:

	//----------------- CONSTRUCTORS ---------------------//

	ServerAnswer(Sockette &ListenSocket);

	//----------------- DESTRUCTOR -----------------------//

	//----------------------- GETTER ---------------------//

	char	*getRequest();

	//----------------- MEMBER FUNCTION ------------------//

	void	readRequest();

};
