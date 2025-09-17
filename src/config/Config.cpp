#include "Config.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Config::Config() : _port(8080), _maxSizeClientRequestBody(__INT_MAX__)
{
	Route r1;
	_routes.push_back(r1);
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
	(void)other;
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

unsigned int Config::getPort() const
{
	return (this->_port);
}

unsigned int Config::getMaxSizeClientRequestBody() const
{
	return (this->_maxSizeClientRequestBody);
}	

std::vector<Route> Config::getRoutes() const
{
	return (this->_routes);
}


//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//
