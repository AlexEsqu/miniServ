#include "Config.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Config::Config() : _port(8080), _maxSizeClientRequestBody(__INT_MAX__)
{
	_routes.push_back(new Route());

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
	if (this != &other)
	{
		_port = other._port;
		_maxSizeClientRequestBody = other._maxSizeClientRequestBody;
		// ...
	}
	return *this;
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

const Route *Config::getRoutes(int index) const
{
	return (this->_routes[index]);
}

//---------------------------- SETTERS --------------------------------------//

//------------------------ MEMBER FUNCTIONS ---------------------------------//
