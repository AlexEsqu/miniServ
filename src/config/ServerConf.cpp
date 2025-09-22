#include "ServerConf.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerConf::ServerConf() : _port(8080), _maxSizeClientRequestBody(__INT_MAX__)
{
	_routes.push_back(new Route("./pages/"));
	_routes.push_back(new Route("./pages/img/"));

#ifdef DEBUG
	std::cout << "ServerConf Constructor called" << std::endl;
#endif
}

ServerConf::ServerConf(const ServerConf &copy)
{
#ifdef DEBUG
	std::cout << "ServerConf copy Constructor called" << std::endl;
#endif
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

ServerConf::~ServerConf()
{
	for (std::vector<Route*>::iterator it = _routes.begin(); it != _routes.end(); it++)
		delete *it;
#ifdef DEBUG
	std::cout << "ServerConf Destructor called" << std::endl;
#endif
}

//---------------------------- OPERATORS ------------------------------------//

ServerConf &ServerConf::operator=(const ServerConf &other)
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

unsigned int ServerConf::getPort() const
{
	return (this->_port);
}

unsigned int ServerConf::getMaxSizeClientRequestBody() const
{
	return (this->_maxSizeClientRequestBody);
}

const Route *ServerConf::getRoutes(int index) const
{
	return (this->_routes[index]);
}

//---------------------------- SETTERS --------------------------------------//

//------------------------ MEMBER FUNCTIONS ---------------------------------//

bool doesFileExist(std::string &requestedFile)
{
	std::ifstream input(requestedFile.c_str()); // opening the file as the content for the response
	std::stringstream content;
	if (!input.is_open())
	{
		std::cerr << RED << "Could not open file " << requestedFile << STOP_COLOR << std::endl;
		return (false);
	}
	return (true);
}

Route *ServerConf::getRootMatchForRequestedFile(std::string &requestedFile) const
{
	std::vector<Route *>::const_iterator it;
	std::string path;
	if (requestedFile[0] == '/') // if the request starts with / the return the first root
		return (*this->_routes.begin());
	for (it = this->_routes.begin(); it != this->_routes.end(); it++)
	{
		path = (*it)->getRootDirectory() + requestedFile;
		if (doesFileExist(path) == true)
		{
			std::cout << GREEN << "FOUND ROOT " << (*it)->getRootDirectory() << STOP_COLOR << std::endl;
			return (*it);
		}
	}
	std::cout << RED << "ROOT NOT FOUND FOR " << requestedFile << STOP_COLOR << std::endl;
	return (*this->_routes.begin());
}
