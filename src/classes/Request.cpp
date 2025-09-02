#include "Request.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Request::Request()
{
	#ifdef DEBUG
		std::cout << "Request Constructor called" << std::endl;
	#endif
}

Request::Request(const Request &copy)
{
	#ifdef DEBUG
		std::cout << "Request copy Constructor called" << std::endl;
	#endif
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

Request::~Request()
{
	#ifdef DEBUG
		std::cout << "Request Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

Request &Request::operator=(const Request &other)
{
	// code
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//


//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//
