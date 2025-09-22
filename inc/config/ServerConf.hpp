#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include "Route.hpp"
#include "readability.hpp"

class Route;

class ServerConf
{

private:

	unsigned int		_port;

	// error pages redirect ?
	unsigned int		_maxSizeClientRequestBody;
	std::vector<Route *>	_routes;


public:

	//----------------- CONSTRUCTORS ---------------------//

	ServerConf();
	ServerConf(std::string &configFilePath);
	ServerConf(const ServerConf &copy);

	//----------------- DESTRUCTOR -----------------------//

	~ServerConf();

	//------------------- OPERATORS ----------------------//

	ServerConf &operator=(const ServerConf &other);


	//--------------- MEMBER FUNCTIONS -------------------//

	unsigned int getPort() const;
	unsigned int getMaxSizeClientRequestBody() const;
	const Route *getRoutes(int index) const;

	Route *getRootMatchForRequestedFile(std::string &requestedFile) const;

};
