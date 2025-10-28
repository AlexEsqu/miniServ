#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <list>
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

	ConfigParser&		operator=(const ConfigParser& other);


	//--------------- MEMBER FUNCTIONS -------------------//

	static std::list<ServerConf>	parseConfigFile(const char* configFilePath);
	static ServerConf	parseServerBlock(std::ifstream& configFileStream);
	static Route		parseLocationBlock(std::ifstream& configFileStream, const std::string& locationLine);
	static std::string	extractLocationPath(const std::string& locationLine);
	static void			addLineAsLocationKeyValue(std::string& line, std::map<std::string, std::string>& paramMap);
	static void			addLineAsServerKeyValue(std::string& line, std::map<std::string, std::string>& paramMap);
	static bool			isClosedCurlyBrace(std::string& line);
	static void			addDefaultRoute(ServerConf& serverConf);
	static void			removeInvalidServers(std::list<ServerConf>& configs);
};
