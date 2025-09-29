#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <fstream>

#include "ServerConf.hpp"
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

	static std::vector<ServerConf>	parseConfigFile(char* configFilePath);
	static ServerConf	parseServerBlock(std::ifstream& configFileStream);
	static void			addLineAsServerKeyValue(std::string& line, std::map<std::string, std::string>& paramMap);
	static bool			isClosedCurlyBrace(std::string& line);

};
