#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include <cstdlib>

#include "Route.hpp"
#include "readability.hpp"
#include "Request.hpp"

class Route;

class ServerConf
{

private:

	unsigned int		_maxSizeClientRequestBody;

	unsigned int		_port;
	std::string			_serverName;
	std::string			_root;

	std::vector<Route>	_routes;
	std::map<std::string, std::string>	_paramMap;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ServerConf();
	ServerConf(std::map<std::string, std::string> paramMap);
	ServerConf(const ServerConf& copy);

	//----------------- DESTRUCTOR -----------------------//

	~ServerConf();

	//------------------- OPERATORS ----------------------//

	ServerConf&					operator=(const ServerConf &other);

	//------------------- GETTERS ------------------------//

	unsigned int				getPort() const;
	unsigned int				getMaxSizeClientRequestBody() const;
	const std::vector<Route>&	getRoutes() const;
	const std::string&			getRoot() const;
	const std::map<std::string, std::string>	&getParamMap() const;

	//------------------- SETTERS ------------------------//

	void						setPort(int portNum);
	void						setServerName(std::string serverName);
	void						setRoot(std::string root);
	void						setParamMap(std::map<std::string, std::string> &paramMap);
	void						addRoute(Route route);

	//--------------- MEMBER FUNCTIONS -------------------//

	Route&						getRoute(std::string path);

};
