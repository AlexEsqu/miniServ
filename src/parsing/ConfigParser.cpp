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


ServerConf*	ConfigParser::parseServerBlock(std::ifstream& configFileStream)
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

		// Handle nested blocks (like location blocks)
		if (line.find("location") != std::string::npos && line[line.size() - 1] == '{')
		{
			// TO DO : parse location block
			// for now skipping the block entirely
			int braceCount = 1;
			while (getline(configFileStream, line) && braceCount > 0) {
				if (line.find('{') != std::string::npos) braceCount++;
				if (line.find('}') != std::string::npos) braceCount--;
				if (braceCount == 0)
					break;
			}

			continue;
		}

		// adds all lines to a map of key setting and value
		addLineAsServerKeyValue(line, paramMap);
	}

	// check the server block is closed
	if (!isClosedCurlyBrace(line))
		throw std::runtime_error("Invalid config file");

	ServerConf* serverConf = new ServerConf;

	if (paramMap.find("listen") != paramMap.end())
		serverConf->setPort(atoi(paramMap["listen"].c_str()));

	// if (paramMap.find("server_name") != paramMap.end())
	// 	serverConf.setServerName(paramMap["server_name"]);

	if (paramMap.find("root") != paramMap.end())
		serverConf->setRoot(paramMap["root"]);

	return serverConf;
}


std::vector<ServerConf*>	ConfigParser::parseConfigFile(char* configFilePath)
{
	std::vector<ServerConf*>	configs;

	if (!configFilePath) {
		configs.push_back(new ServerConf());
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
