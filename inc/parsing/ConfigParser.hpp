#pragma once
#include <iostream>
#include <vector>
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

	static std::vector<ServerConf>	parseArg(int argc, char** argv);
	static std::vector<ServerConf>	parseConfigFile(const char* configFilePath);

	static ServerConf	parseServerBlock(std::ifstream& configFileStream);
	static Route		parseLocationBlock(std::ifstream& configFileStream, const std::string& locationLine);

	static std::string	extractLocationPath(const std::string& locationLine);
	static void			addLineAsLocationKeyValue(std::string& line, std::map<std::string, std::string>& paramMap);
	static void			addLineAsServerKeyValue(std::string& line, std::map<std::string, std::string>& paramMap);
	static bool			isClosedCurlyBrace(std::string& line);
	static void			addDefaultRoute(ServerConf& serverConf);

	static bool			isValidIPAddress(const std::string& ip);
	static bool			isValidPort(const std::string& port);
	static void			addPortAndIpAddress(std::string& line, std::map<std::string, std::string>& paramMap);
	static void			addPort(std::string line, std::map<std::string, std::string>& paramMap);
	static void			addIpAddress(std::string line, std::map<std::string, std::string>& paramMap);

	static void			removeInvalidServerConf(std::vector<ServerConf>& configs);
};
