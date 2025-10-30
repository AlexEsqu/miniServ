#include "ConfigParser.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ConfigParser::ConfigParser()
{
	verboseLog("ConfigParser Constructor called");
}

ConfigParser::ConfigParser(const ConfigParser &copy)
{
	verboseLog("ConfigParser copy Constructor called");
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

ConfigParser::~ConfigParser()
{
	verboseLog("ConfigParser Destructor called");
}

//---------------------------- OPERATORS ------------------------------------//

ConfigParser	&ConfigParser::operator=(const ConfigParser &)
{
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//

//---------------------------- SETTERS --------------------------------------//

//------------------------ MEMBER FUNCTIONS ---------------------------------//

bool	ConfigParser::isClosedCurlyBrace(std::string &line)
{
	if (trim(line) == "}")
		return true;
	return false;
}

void	ConfigParser::addLineAsServerKeyValue(std::string &line, std::map<std::string, std::string> &paramMap)
{
	// removes leading/trailing whitespace
	line = trim(line);

	// skips empty lines
	if (line.empty())
		return;

	// find the first space or tab (separator between key and value)
	size_t pos = line.find_first_of(" \t");
	if (pos != std::string::npos)
	{
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);
		trim(value);

		// Remove semicolon from value if present
		if (!value.empty() && value[value.size() - 1] == ';')
		{
			value.erase(value.size() - 1);
			trim(value);
		}

		paramMap[key] = value;
	}
}

std::string	ConfigParser::extractLocationPath(const std::string &locationLine)
{
	size_t start = locationLine.find("location") + 8;
	size_t end = locationLine.find('{');

	if (start >= end)
		throw std::invalid_argument("Invalid location directive");

	std::string pathPart = locationLine.substr(start, end - start);
	trim(pathPart);

	return pathPart;
}

void	ConfigParser::addLineAsLocationKeyValue(std::string &line, std::map<std::string, std::string> &paramMap)
{
	line = trim(line);
	if (line.empty())
		return;

	size_t pos = line.find_first_of(" \t");
	if (pos != std::string::npos)
	{
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);
		trim(value);

		if (!value.empty() && value[value.size() - 1] == ';')
		{
			value.erase(value.size() - 1);
			trim(value);
		}
		paramMap[key] = value;
	}
}

Route	ConfigParser::parseLocationBlock(std::ifstream &configFileStream, const std::string &locationLine)
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
		throw std::invalid_argument("Invalid location block - missing closing brace");

	route.setRouteParam(paramMap);

	return route;
}

// Makes sure the base server has the basic necessary rules such as a root, an index, and methods
void	ConfigParser::addDefaultRoute(ServerConf &serverConf)
{
	Route defaultRoute;

	defaultRoute.setURLPath("/");

	std::map<std::string, std::string> defaultParams;

	if (serverConf.getRoot().empty())
		defaultParams["root"] = "/var/www/html";
	else
		defaultParams["root"] = serverConf.getRoot();

	if (serverConf.getParamMap().find("index") != serverConf.getParamMap().end())
		defaultParams["index"] = serverConf.getParamMap().at("index");

	if (serverConf.getParamMap().find("allowed_methods") == serverConf.getParamMap().end()) // if map entry == map.end(), it doesn't exist in the map; HOWEVER map[entry] can return a value and create an entry if none existed
		defaultParams["allowed_methods"] = "GET HEAD";
	else
		defaultParams["allowed_methods"] = serverConf.getParamMap().at("allowed_methods");

	if (serverConf.getParamMap().find("autoindex") != serverConf.getParamMap().end())
		defaultParams["autoindex"] = serverConf.getParamMap().at("autoindex");

	if (serverConf.getParamMap().find("upload") == serverConf.getParamMap().end())
		defaultParams["upload"] = "pages/upload";
	else
		defaultParams["upload"] = serverConf.getParamMap().at("upload");

	// Apply the default parameters to the route
	defaultRoute.setRouteParam(defaultParams);

	// Add the default route to the server configuration
	serverConf.addRoute(defaultRoute);
}

ServerConf ConfigParser::parseServerBlock(std::ifstream &configFileStream)
{
	std::map<std::string, std::string> paramMap;
	std::vector<Route> routes;

	// goes through the server config block until the closing bracket
	std::string line;
	while (getline(configFileStream, line) && !isClosedCurlyBrace(line))
	{
		// Remove whitespace
		trim(line);

		// ignores empty line and comments
		if (line.empty() || ltrim(line)[0] == '#')
			continue;

		// handle nested location blocks
		if (line.find("location") != std::string::npos && line[line.size() - 1] == '{')
		{
			Route route = parseLocationBlock(configFileStream, line);
			routes.push_back(route);
		}

		// adds all lines to a map of key setting and value
		else
			addLineAsServerKeyValue(line, paramMap);
	}

	// check the server block is closed
	if (!isClosedCurlyBrace(line))
		throw std::invalid_argument("Invalid config file: no closing curly brace");

	addPortAndIpAddress(paramMap["listen"], paramMap);

	ServerConf serverConf(paramMap);

	#ifdef DEBUG
	std::cout << "Config block parsed :\n";
	for (std::map<std::string, std::string>::iterator it = paramMap.begin(); it != paramMap.end(); it++)
		std::cout << "[" << it->first << "] = [" << it->second << "]\n";
	#endif

	for (size_t i = 0; i < routes.size(); i++)
		serverConf.addRoute(routes[i]);

	addDefaultRoute(serverConf);

	return serverConf;
}

void					ConfigParser::removeInvalidServerConf(std::vector<ServerConf>& configs)
{
	std::set<int>			setOfUsedPorts;
	std::set<std::string>	setOfUsedHostName;

	for (std::vector<ServerConf>::iterator i = configs.begin(); i != configs.end(); i++)
	{
		if (setOfUsedPorts.find(i->getPort()) != setOfUsedPorts.end())
		{
			std::cout << "Server " << i->getServerName() << " is using another server's port, deleting...\n";
			configs.erase(i--);
			continue;
		}
		else
			setOfUsedPorts.insert(i->getPort());

		if (setOfUsedHostName.find(i->getServerName()) != setOfUsedHostName.end())
		{
			std::cout << "Server " << i->getServerName() << " is using another server's name, deleting...\n";
			configs.erase(i--);
			continue;
		}
		else
			setOfUsedHostName.insert(i->getServerName());
	}
}

std::vector<ServerConf>	ConfigParser::parseConfigFile(const char *configFilePath)
{
	std::vector<ServerConf> configs;

	if (!configFilePath)
	{
		configs.push_back(ServerConf());
		return configs;
	}

	std::ifstream configFileStream(configFilePath);
	if (!configFileStream)
		throw std::invalid_argument("Failed to read config file");

	std::string line;
	while (getline(configFileStream, line))
	{
		line = trim(line);

		// skips comment and empty lines
		if (line.empty() || line[0] == '#')
			continue;

		// if a line starts with server and ends with {, initiate parse server block
		if (line.rfind("server", 0) != std::string::npos && line[line.size() - 1] == '{')
		{
			configs.push_back(parseServerBlock(configFileStream));
		}
	}

	removeInvalidServerConf(configs);

	if (configs.empty())
		throw std::invalid_argument("No valid configurations in config file");

	return configs;
}

std::vector<ServerConf>	ConfigParser::parseConfig(int argc, char** argv)
{
	if (argc != 2)
	{
		throw std::invalid_argument("Usage: ./webserv configuration_file");
	}

	std::ifstream configFile(argv[1]);
	if (!configFile.is_open())
	{
		throw std::invalid_argument("The configuration file could not be opened");
	}
	configFile.close();

	return parseConfigFile(argv[1]);
}

// check the provided IP address make sense
bool	ConfigParser::isValidIPAddress(const std::string& ip)
{
	std::istringstream ipStream(ip);
	std::string segment;
	int count = 0;

	while (std::getline(ipStream, segment, '.'))
	{
		if (++count > 4)
			return false;
		if (segment.empty() || segment.size() > 3)
			return false;
		for (size_t i = 0; i < segment.size(); i++)
		{
			if (!isdigit(segment[i]))
				return false;
		}
		int num = std::atoi(segment.c_str());
		if (num < 0 || num > 255)
			return false;
		if (segment[0] == '0' && segment.size() > 1)
			return false;
	}

	return (count == 4);
}

// check the provided port is possible
bool	ConfigParser::isValidPort(const std::string& portStr)
{
	if (portStr.empty() || portStr.size() > 5)
		return false;

	for (size_t i = 0; i < portStr.size(); i++)
	{
		if (!isdigit(portStr[i]))
			return false;
	}

	int port = std::atoi(portStr.c_str());

	return port > 0 && port <= 65535;
}

void	ConfigParser::addPort(std::string line, std::map<std::string, std::string>& paramMap)
{
	if (!isValidPort(line))
		throw std::invalid_argument("Invalid port: '" + line + "'");

	paramMap["port"] = line;
}

void	ConfigParser::addIpAddress(std::string line, std::map<std::string, std::string>& paramMap)
{
	if (!isValidIPAddress(line))
		throw std::invalid_argument("Invalid IP address: '" + line + "'");

	paramMap["interface"] = line;
}

// parses, validates and add either the lone port, or an ip address and its port, both as a string to param map
void	ConfigParser::addPortAndIpAddress(std::string& line, std::map<std::string, std::string>& paramMap)
{
	size_t colonPos = line.find(':');
	if (colonPos == std::string::npos)
	{
		addPort(line, paramMap);
		return;
	}

	else
	{
		addPort(line.substr(colonPos + 1), paramMap);
		addIpAddress(line.substr(0, colonPos), paramMap);
	}
}
