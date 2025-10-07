#include "ConfigParser.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ConfigParser::ConfigParser()
{
	#ifdef DEBUG
		std::cout << "ConfigParser Constructor called" << std::endl;
	#endif
}

ConfigParser::ConfigParser(const ConfigParser& copy)
{
	#ifdef DEBUG
		std::cout << "ConfigParser copy Constructor called" << std::endl;
	#endif
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

ConfigParser::~ConfigParser()
{
	#ifdef DEBUG
		std::cout << "ConfigParser Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

ConfigParser& ConfigParser::operator=(const ConfigParser&)
{
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//


//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//

bool	ConfigParser::isClosedCurlyBrace(std::string& line)
{
	if (trim(line) == "}")
		return true;
	return false;
}

void	ConfigParser::addLineAsServerKeyValue(std::string& line, std::map<std::string, std::string>& paramMap)
{
	// removes leading/trailing whitespace
	line = trim(line);

	// skips empty lines
	if (line.empty())
		return;

	// find the first space or tab (separator between key and value)
	size_t pos = line.find_first_of(" \t");
	if (pos != std::string::npos) {
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);
		trim(value);

		// Remove semicolon from value if present
		if (!value.empty() && value[value.size() - 1] == ';') {
			value.erase(value.size() - 1);
			trim(value);
		}

		paramMap[key] = value;
	}
}

std::string ConfigParser::extractLocationPath(const std::string& locationLine)
{
	size_t start = locationLine.find("location") + 8;
	size_t end = locationLine.find('{');

	if (start >= end)
		throw std::runtime_error("Invalid location directive");

	std::string pathPart = locationLine.substr(start, end - start);
	trim(pathPart);

	return pathPart;
}

void ConfigParser::addLineAsLocationKeyValue(std::string& line, std::map<std::string, std::string>& paramMap)
{
	line = trim(line);
	if (line.empty())
		return;

	size_t pos = line.find_first_of(" \t");
	if (pos != std::string::npos) {
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);
		trim(value);

		if (!value.empty() && value[value.size() - 1] == ';') {
			value.erase(value.size() - 1);
			trim(value);
		}
		paramMap[key] = value;
	}
}

Route	ConfigParser::parseLocationBlock(std::ifstream& configFileStream, const std::string& locationLine)
{
	Route route;
	std::map<std::string, std::string> paramMap;

	std::string path = extractLocationPath(locationLine);
	route.setURLPath(path);

	std::string line;
	while (getline(configFileStream, line) && !ConfigParser::isClosedCurlyBrace(line))
	{
		trim(line);

		if (line.empty() || ltrim(line)[0] == '#')
			continue;

		// recurse on nested location blocks
		if (line.find("location") != std::string::npos && line[line.size() - 1] == '{')
		{
			class Route nestedLocation = parseLocationBlock(configFileStream, line);
			route.addNestedRoute(nestedLocation);
			continue;
		}

		addLineAsLocationKeyValue(line, paramMap);
	}

	if (!isClosedCurlyBrace(line))
		throw std::runtime_error("Invalid location block - missing closing brace");

	route.setRouteParam(paramMap);

	return route;
}


void ConfigParser::addDefaultRoute(ServerConf& serverConf)
{
	Route defaultRoute;

	defaultRoute.setURLPath("/");

	std::map<std::string, std::string> defaultParams;

	defaultParams["root"] = serverConf.getRoot().empty() ? "/var/www/html" : serverConf.getRoot();
	defaultParams["index"] = "index.html";
	defaultParams["allow_methods"] = "GET POST DELETE";
	defaultParams["autoindex"] = "off";

	// Apply the default parameters to the route
	defaultRoute.setRouteParam(defaultParams);

	// Add the default route to the server configuration
	serverConf.addRoute(defaultRoute);

}

ServerConf	ConfigParser::parseServerBlock(std::ifstream& configFileStream)
{
	std::map<std::string, std::string>	paramMap;

	// goes through the server config block until the closing bracket
	std::string	line;
	while (getline(configFileStream, line) && !isClosedCurlyBrace(line))
	{
		// Remove whitespace
		trim(line);

		// ignores empty line and comments
		if (line.empty() || ltrim(line)[0] == '#')
			continue;

		// handle nested location blocks
		if (line.find("location") != std::string::npos && line[line.size() - 1] == '{')
			 parseLocationBlock(configFileStream, line);

		// adds all lines to a map of key setting and value
		addLineAsServerKeyValue(line, paramMap);
	}

	// check the server block is closed
	if (!isClosedCurlyBrace(line))
		throw std::runtime_error("Invalid config file: no closing curly brace");

	ServerConf serverConf;

	if (paramMap.find("listen") != paramMap.end())
		serverConf.setPort(atoi(paramMap["listen"].c_str()));

	if (paramMap.find("server_name") != paramMap.end())
		serverConf.setServerName(paramMap["server_name"]);

	if (paramMap.find("root") != paramMap.end())
	{
		serverConf.setRoot(paramMap["root"]);
		addDefaultRoute(serverConf);
	}


	std::cout << "Config block parsed :\n";
	for (std::map<std::string, std::string>::iterator it = paramMap.begin(); it != paramMap.end(); it++)
	{
		std::cout << "[" << it->first << "] = [" << it->second << "]\n";
	}

	return serverConf;
}


std::vector<ServerConf>	ConfigParser::parseConfigFile(const char* configFilePath)
{
	std::vector<ServerConf>	configs;

	if (!configFilePath) {
		configs.push_back(ServerConf());
		return configs;
	}

	std::ifstream	configFileStream(configFilePath);
	if (!configFileStream)
		throw std::runtime_error("Failed to read config file");

	std::string	line;
	while (getline(configFileStream, line))
	{
		line = trim(line);

		// skips comment and empty lines
		if (line.empty() || line[0] == '#')
			continue;

		// if a line starts with server and ends with {, initiate parse server block
		if (line.rfind("server", 0) != std::string::npos && line[line.size() - 1] == '{') {
			configs.push_back(parseServerBlock(configFileStream));
		}
	}

	return configs;
}
