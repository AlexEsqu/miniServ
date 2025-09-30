#include "ServerConf.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerConf::ServerConf()
	: _port(8080)
	, _maxSizeClientRequestBody(__INT_MAX__)
{
	_routes.push_back(new Route("./pages/"));
	_routes.push_back(new Route("./pages/img/"));

#ifdef DEBUG
	std::cout << "ServerConf Generic Constructor called" << std::endl;
#endif
}

ServerConf::ServerConf(std::map<std::string, std::string> paramMap, std::vector<Route*> routes)
	: _port(8080)
	, _maxSizeClientRequestBody(__INT_MAX__)
	, _routes(routes)
	, _paramMap(paramMap)
{
	// Initialize _root from paramMap
	if (_paramMap.find("root") != _paramMap.end()) {
		_root = _paramMap["root"];
	}

	// Initialize _port from paramMap
	if (_paramMap.find("listen") != _paramMap.end()) {
		_port = atoi(_paramMap["listen"].c_str());
	}


#ifdef DEBUG
	std::cout << "ServerConf Custom Constructor called" << std::endl;
#endif
}

ServerConf::ServerConf(const ServerConf &copy)
	: _port(copy._port)
	, _maxSizeClientRequestBody(copy._maxSizeClientRequestBody)
	, _root(copy._root)
	, _paramMap(copy._paramMap)
{
	for (size_t i = 0; i < copy._routes.size(); ++i)
		_routes.push_back(new Route(*copy._routes[i]));

#ifdef DEBUG
	std::cout << "ServerConf copy Constructor called" << std::endl;
#endif
}

//--------------------------- DESTRUCTORS -----------------------------------//

ServerConf::~ServerConf()
{
	for (std::vector<Route*>::iterator it = _routes.begin(); it != _routes.end(); ++it)
		delete *it; // Free the Route objects
	_routes.clear();
#ifdef DEBUG
	std::cout
		<< "ServerConf Destructor called" << std::endl;
#endif
}

//---------------------------- OPERATORS ------------------------------------//

ServerConf&		ServerConf::operator=(const ServerConf &other)
{
	if (this != &other)
	{
		_port = other._port;
		_maxSizeClientRequestBody = other._maxSizeClientRequestBody;
		_root = other._root;
		_paramMap = other._paramMap;

		// deep copy of the routes, so inefficient
		for (size_t i = 0; i < other._routes.size(); ++i)
			_routes.push_back(new Route(*other._routes[i]));
	}
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

unsigned int		ServerConf::getPort() const
{
	return (_port);
}

unsigned int		ServerConf::getMaxSizeClientRequestBody() const
{
	return (_maxSizeClientRequestBody);
}

const Route*		ServerConf::getRoutes(int index) const
{
	return (_routes[index]);
}

const std::string&	ServerConf::getRoot() const
{
	return (_root);
}

//---------------------------- SETTERS --------------------------------------//

void			ServerConf::setPort(int portNum)
{
	_port = portNum;
}

void			ServerConf::setRoot(std::string root)
{
	_root = root;
}

void			ServerConf::setParamMap(std::map<std::string, std::string>& paramMap)
{
	_paramMap = paramMap;
}

void			ServerConf::setServerName(std::string serverName)
{
	_serverName = serverName;
}

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
			return (*it);
	}
	return (*this->_routes.begin());
}
