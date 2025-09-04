#pragma once
# include "Sockette.hpp"


class SocketteAnswer: public Sockette
{

private:

	char	_buffer[30000];

public:

	//----------------- CONSTRUCTORS ---------------------//

	SocketteAnswer(Sockette &Source);

	//----------------- DESTRUCTOR -----------------------//

	//----------------------- GETTER ---------------------//

	char	*getRequest();

	//----------------- MEMBER FUNCTION ------------------//

	void	readRequest();

};
