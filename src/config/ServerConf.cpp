#include "ServerConf.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerConf::ServerConf()
	: _port(DEFAULT_PORT)
	, _maxSizeClientRequestBody(DEFAULT_MAX_BODY_SIZE)
{

#ifdef DEBUG
	std::cout << "ServerConf Generic Constructor called" << std::endl;
#endif
}

ServerConf::ServerConf(std::map<std::string, std::string> paramMap)
	: _port(DEFAULT_PORT)
	, _maxSizeClientRequestBody(DEFAULT_MAX_BODY_SIZE)
	, _paramMap(paramMap)
{
	setPort(paramMap["port"]);
	setServerName(paramMap["server_name"]);
	setRoot(paramMap["root"]);
	setMaxBodySize(paramMap["client_max_body_size"]);

#ifdef DEBUG
	std::cout << "ServerConf Custom Constructor called" << std::endl;
#endif
}

ServerConf::ServerConf(const ServerConf &copy)
	: _port(copy._port)
	, _serverName(copy._serverName)
	, _root(copy._root)
	, _maxSizeClientRequestBody(copy._maxSizeClientRequestBody)
	, _paramMap(copy._paramMap)
{
	_routes.clear();
	_routes.reserve(copy._routes.size());
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
	std::cout << "ServerConf Destructor called" << std::endl;
#endif
}

//---------------------------- OPERATORS ------------------------------------//

ServerConf&		ServerConf::operator=(const ServerConf &other)
{
	if (this != &other)
	{
		_port = other._port;
		_serverName = other._serverName;
		_root = other._root;
		_maxSizeClientRequestBody = other._maxSizeClientRequestBody;

		_paramMap = other._paramMap;

		_routes.clear();
		_routes.reserve(other._routes.size());
		for (size_t i = 0; i < other._routes.size(); i++)
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

const std::vector<Route>&	ServerConf::getRoutes() const
{
	return (_routes);
}

const std::string&			ServerConf::getRoot() const
{
	return (_root);
}

const std::string			ServerConf::getServerName() const
{
	return (_serverName);
}

const std::map<std::string, std::string>	&ServerConf::getParamMap() const
{
	return (_paramMap);
}

std::map<size_t, Session>& ServerConf::getSessionMap()
{
	return _sessionMap;
}

//---------------------------- SETTERS --------------------------------------//

void			ServerConf::setPort(const std::string& portString)
{
	if (portString.empty())
		std::invalid_argument("no port in server block");
	_port = atoi(portString.c_str());
}

void			ServerConf::setRoot(const std::string& root)
{
	if (root.empty())
		_root = DEFAULT_ROOT;
	else
		_root = root;
}

void			ServerConf::setServerName(const std::string& serverName)
{
	if (serverName.empty())
		std::invalid_argument("no server name in server block");
	_serverName = serverName;
}

void			ServerConf::setMaxBodySize(const std::string& maxBodySize)
{
	if (maxBodySize.empty())
		_maxSizeClientRequestBody = DEFAULT_MAX_BODY_SIZE;
	else
		_maxSizeClientRequestBody = atoi(maxBodySize.c_str());
}

// void			ServerConf::setParamMap(std::map<std::string, std::string>& paramMap)
// {
// 	_paramMap = paramMap;

// 	setPort(paramMap["port"]);
// 	setServerName(paramMap["server_name"]);
// 	setRoot(paramMap["root"]);
// 	setMaxBodySize(paramMap["client_max_body_size"]);
// }

void			ServerConf::addRoute(Route route)
{
	_routes.push_back(route);
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//

