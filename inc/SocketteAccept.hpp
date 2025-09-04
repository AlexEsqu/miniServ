#pragma once
# include "Sockette.hpp"


class SocketteAccept: public Sockette
{

private:

public:

	//----------------- CONSTRUCTORS ---------------------//

	SocketteAccept(Sockette &Source);

	//----------------- DESTRUCTOR -----------------------//

	~SocketteAccept();


};
