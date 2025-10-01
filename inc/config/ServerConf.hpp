#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include <cstdlib>

#include "Route.hpp"
#include "readability.hpp"

class Route;

class ServerConf
{

private:

	unsigned int		_port;
	std::string			_serverName;
	unsigned int		_maxSizeClientRequestBody;
	std::string			_root;
	std::vector<Route>	_routes;
	std::map<std::string, std::string>	_paramMap;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ServerConf();
	ServerConf(std::map<std::string, std::string> paramMap, std::vector<Route> routes);
	ServerConf(const ServerConf& copy);

	//----------------- DESTRUCTOR -----------------------//

	~ServerConf();

	//------------------- OPERATORS ----------------------//

	ServerConf&			operator=(const ServerConf &other);

	//------------------- GETTERS ------------------------//

	unsigned int		getPort() const;
	unsigned int		getMaxSizeClientRequestBody() const;
	const Route			getRoutes(int index) const;
	const std::string&	getRoot() const;

	//------------------- SETTERS ------------------------//

	void				setPort(int portNum);
	void				setServerName(std::string serverName);
	void				setRoot(std::string root);
	void				setParamMap(std::map<std::string, std::string> &paramMap);
	void				addRoute(Route route);

	//--------------- MEMBER FUNCTIONS -------------------//

	std::string			getRoutedURL(std::string &requestedFile);

};
