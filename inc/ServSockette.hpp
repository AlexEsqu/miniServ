#pragma once
# include "Sockette.hpp"

// Sockette inheriting class to centralize setup for server socket
// constructor includes binding to IP address and listening
class ServSockette: public Sockette
{

private:

public:

	//----------------- CONSTRUCTORS ---------------------//

	ServSockette(int port);

	//----------------- DESTRUCTOR -----------------------//

	~ServSockette();

	//------------------- OPERATORS ----------------------//


};
