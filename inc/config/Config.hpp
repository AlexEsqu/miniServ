#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#include "Route.hpp"
#include "readability.hpp"

class Route;

class Config
{

private:

	unsigned int		_port;

	// error pages redirect ?
	unsigned int		_maxSizeClientRequestBody;
	std::vector<Route *>	_routes;


public:

	//----------------- CONSTRUCTORS ---------------------//

	Config();
	Config(std::string &configFilePath);
	Config(const Config &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Config();

	//------------------- OPERATORS ----------------------//

	Config &operator=(const Config &other);


	//--------------- MEMBER FUNCTIONS -------------------//

	unsigned int getPort() const;
	unsigned int getMaxSizeClientRequestBody() const;
	const Route *getRoutes(int index) const;

	Route *getRootMatchForRequestedFile(std::string &requestedFile) const;

};
