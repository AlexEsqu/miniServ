#pragma once
#include <iostream>
#include <vector>

#include "Config.hpp"
#include "parsing.hpp"

class ConfigParser
{

private:

public:

	//----------------- CONSTRUCTORS ---------------------//

	ConfigParser();
	ConfigParser(const ConfigParser& copy);

	//----------------- DESTRUCTOR -----------------------//

	~ConfigParser();

	//------------------- OPERATORS ----------------------//

	ConfigParser& operator=(const ConfigParser& other);


	//--------------- MEMBER FUNCTIONS -------------------//

	static std::vector<Config*>	readConfigs(std::string& configFilePath);

};
