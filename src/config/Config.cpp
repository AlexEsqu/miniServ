#include "Config.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Config::Config()
{
	#ifdef DEBUG
		std::cout << "Config Constructor called" << std::endl;
	#endif
}

Config::Config(const Config &copy)
{
	#ifdef DEBUG
		std::cout << "Config copy Constructor called" << std::endl;
	#endif
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

Config::~Config()
{
	#ifdef DEBUG
		std::cout << "Config Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

Config &Config::operator=(const Config &other)
{
	// code
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//


//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//
