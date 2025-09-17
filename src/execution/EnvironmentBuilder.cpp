#include "EnvironmentBuilder.hpp"
#include "parsing.hpp"
#include "Request.hpp"

//----------------- CONSTRUCTORS ---------------------//

EnvironmentBuilder::EnvironmentBuilder()
{

}

EnvironmentBuilder::EnvironmentBuilder(const EnvironmentBuilder &original)
{
	*this = original;
}

//----------------- DESTRUCTOR -----------------------//

EnvironmentBuilder::~EnvironmentBuilder()
{

}

//------------------- OPERATORS ----------------------//

EnvironmentBuilder& EnvironmentBuilder::operator=(const EnvironmentBuilder &original)
{
	if (this == &original)
		return *this;

	_requestEnvMap = original._requestEnvMap;
	return *this;
}

//------------------- SETTERS ------------------------//

void	EnvironmentBuilder::cutFormatAddToEnv(std::string& keyValueString)
{
	size_t equalPos = keyValueString.find(':');
	if (equalPos != std::string::npos) {
		std::string key = keyValueString.substr(0, equalPos);
		std::string value = keyValueString.substr(equalPos + 1);
		key = trim(key);
		value = trim(value);
		setAsHTTPVariable(key, value);
	}
}

void	EnvironmentBuilder::setAsEnv(const std::string& key, const std::string& value)
{
	_requestEnvMap[key] = value;
}

void EnvironmentBuilder::setAsHTTPVariable(const std::string& key, const std::string& value)
{
	std::string	formattedKey;

	// Replace hyphens with underscores and convert to uppercase
	for (size_t i = 0; i < key.length(); ++i) {
		if (key[i] == '-' || key[i] == ' ') {
			formattedKey[i] = '_';
		}
	}
	strToUpper(formattedKey);

	// Convert HTTP headers to CGI format: HTTP_HEADER_NAME
	if (formattedKey != "CONTENT-TYPE" && formattedKey != "CONTENT-LENGTH")
		std::string cgiName = "HTTP_" + formattedKey;

	setAsEnv(formattedKey, value);
}

void EnvironmentBuilder::setupCGIEnvironment(const Request& request)
{
	// Standard CGI variables
	setAsEnv("REQUEST_METHOD", request.getMethod());
	setAsEnv("REQUEST_URI", request.getRequestedURL());
	setAsEnv("SERVER_PROTOCOL", request.getProtocol());

	// Server information
	setAsEnv("SERVER_NAME", "localhost");		// or from config
	setAsEnv("SERVER_PORT", "8080");			// or from config
}

//------------------- GETTERS ------------------------//

Environment	EnvironmentBuilder::getPHPEnv()
{
	std::vector<std::string>	envAsString;

	std::map<std::string, std::string>::const_iterator item;
	for (item = _requestEnvMap.begin(); item != _requestEnvMap.end(); item++)
	{
		std::string envVar = item->first + "=" + item->second;
		envAsString.push_back(envVar);
	}

	Environment	env(envAsString);
	return env;
}

// Environment	EnvironmentBuilder::getPythonEnv()
// {
// 	char** pythonEnv = new char*[_requestEnvMap.size() + 1];

// 	size_t index = 0;
// 	std::map<std::string, std::string>::const_iterator item;
// 	for (item = _requestEnvMap.begin();
// 			item != _requestEnvMap.end();
// 			++item)
// 	{
// 		std::string envVar = item->first + "=" + item->second;
// 		pythonEnv[index] = new char[envVar.length() + 1];
// 		std::strcpy(pythonEnv[index], envVar.c_str());
// 		index++;
// 	}

// 	pythonEnv[index] = NULL;

// 	return pythonEnv;
// }

std::string	EnvironmentBuilder::getSpecificEnv(std::string& key)
{
	std::map<std::string, std::string>::iterator it;

	it = _requestEnvMap.find(key);
	if (it == _requestEnvMap.end())
		return "";

	return it->first + "=" + it->second;
}
