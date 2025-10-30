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
#include "Session.hpp"

class Route;

#define	DEFAULT_ROOT "pages"
#define DEFAULT_MAX_BODY_SIZE 1
#define DEFAULT_DEFAULT_PAGE "index.html"
#define DEFAULT_PORT 8080

class ServerConf
{

private:

	unsigned int		_port;
	std::string			_serverName;
	std::string			_root;
	unsigned int		_maxSizeClientRequestBody;

	std::vector<Route>	_routes;
	std::map
		<std::string,
		std::string>	_paramMap;
	std::map
		<size_t,
		Session>		_sessionMap;

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
	const std::string			getServerName() const;
	unsigned int				getMaxSizeClientRequestBody() const;
	const std::vector<Route>&	getRoutes() const;
	const std::string&			getRoot() const;
	const std::map
		<std::string,
		std::string>&			getParamMap() const;
	std::map<size_t, Session>&	getSessionMap();

	//------------------- SETTERS ------------------------//

	void						setPort(const std::string& port);
	void						setServerName(const std::string& serverName);
	void						setMaxBodySize(const std::string& maxBodySize);
	void						setRoot(const std::string& root);
	void						setParamMap(std::map<std::string, std::string> &paramMap);
	void						addRoute(Route route);

	//--------------- MEMBER FUNCTIONS -------------------//

	Route&						getRoute(std::string path);

};
