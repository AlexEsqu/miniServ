#include "ServerConf.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerConf::ServerConf()
	: _port(8080)
	, _maxSizeClientRequestBody(__INT_MAX__)
{

#ifdef DEBUG
	std::cout << "ServerConf Generic Constructor called" << std::endl;
#endif
}

ServerConf::ServerConf(std::map<std::string, std::string> paramMap, std::vector<Route> routes)
	: _port(8080)
	, _maxSizeClientRequestBody(__INT_MAX__)
	, _routes(routes)
	, _paramMap(paramMap)
{
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
	, _paramMap(copy._paramMap)
{
	for (size_t i = 0; i < copy._routes.size(); ++i)
		_routes.push_back(Route(copy._routes[i]));

#ifdef DEBUG
	std::cout << "ServerConf copy Constructor called" << std::endl;
#endif
}

//--------------------------- DESTRUCTORS -----------------------------------//

ServerConf::~ServerConf()
{
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
		_paramMap = other._paramMap;
		for (size_t i = 0; i < other._routes.size(); ++i)
			_routes.push_back(Route(other._routes[i]));
	}
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

unsigned int				ServerConf::getPort() const
{
	return (_port);
}

unsigned int				ServerConf::getMaxSizeClientRequestBody() const
{
	return (_maxSizeClientRequestBody);
}

const std::vector<Route>	ServerConf::getRoutes() const
{
	return (_routes);
}

const std::string&			ServerConf::getRoot() const
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

void			ServerConf::addRoute(Route route)
{
	_routes.push_back(route);
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//

bool	doesFileExist(std::string &requestedFile)
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
