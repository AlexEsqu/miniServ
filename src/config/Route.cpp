#include "Route.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Route::Route()
{
	#ifdef DEBUG
		std::cout << "Route Constructor called" << std::endl;
	#endif
}

Route::Route(const Route &copy)
{
	#ifdef DEBUG
		std::cout << "Route copy Constructor called" << std::endl;
	#endif
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

Route::~Route()
{
	#ifdef DEBUG
		std::cout << "Route Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

Route &Route::operator=(const Route &other)
{
	// code
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//


//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//
