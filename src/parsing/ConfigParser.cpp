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

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
	// code
	return (*this);
}

//---------------------------- GUETTERS -------------------------------------//


//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//


void	parseServerBlock(std::vector<Config*> dest, std::ifstream& configFileStream, std::string line)
{
	if (line.find("server") != std::string::npos)
	{

	}
}


std::vector<Config*>	ConfigParser::readConfigs(std::string& configFilePath)
{
	std::ifstream	configFileStream(configFilePath);
	if (!configFileStream)
		throw std::runtime_error("Failed to read config file");

	std::vector<Config*>	configs;

	std::string	line;
	while (getline(configFileStream, line))
	{
		if (ltrim(line).substr(0, 7) == "server")
			parseServerBlock(configs, configFileStream, line);
	}

}
