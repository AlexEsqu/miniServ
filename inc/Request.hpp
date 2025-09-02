#pragma once
#include <iostream>

class Request
{

private:

public:

	//----------------- CONSTRUCTORS ---------------------//

	Request();
	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//------------------- OPERATORS ----------------------//

	Request &operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

};
