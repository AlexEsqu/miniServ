#pragma once
#include <iostream>
#include <vector>

class Route;

class Config
{

private:

	unsigned int		_port;
	std::string			_host;
	std::string			_serverName;
	// error pages redirect ?
	unsigned int		_maxSizeClientRequestBody;
	std::vector<Route>	_routes;


public:

	//----------------- CONSTRUCTORS ---------------------//

	Config();
	Config(const Config &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Config();

	//------------------- OPERATORS ----------------------//

	Config &operator=(const Config &other);


	//--------------- MEMBER FUNCTIONS -------------------//

};
