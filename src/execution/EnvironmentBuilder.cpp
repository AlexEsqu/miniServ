#include "EnvironmentBuilder.hpp"
#include "parsing.hpp"
#include "Request.hpp"

//----------------- CONSTRUCTORS ---------------------//

EnvironmentBuilder::EnvironmentBuilder(Request& req)
	: _envAsMap(req.getAdditionalHeaderInfo())
	, _request(req)
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

	_request = original._request;
	return *this;
}

//------------------- SETTERS ------------------------//

void	EnvironmentBuilder::setAsEnv(const std::string& key, const std::string& value)
{
	_envAsMap[key] = value;
	#ifdef DEBUG
		std::cout << "adding env var as [" << key << " = " << value << "]\n";
	#endif
}

//------------------- GETTERS ------------------------//

std::string	EnvironmentBuilder::getSpecificEnv(std::string& key)
{
	std::map<std::string, std::string>::iterator it;

	it = _envAsMap.find(key);
	if (it == _envAsMap.end())
		return "";

	return it->first + "=" + it->second;
}

//------------------- PHP ------------------------//

Environment	EnvironmentBuilder::generatePHPEnv()
{
	std::vector<std::string>	envAsStrVec;

	addCGIEnvironment(envAsStrVec, _request);

	std::map<std::string, std::string>::const_iterator item;
	for (item = _envAsMap.begin(); item != _envAsMap.end(); item++)
	{
		envAsStrVec.push_back(formatAsHTTPVariable(item->first, item->second));
	}

	Environment	env(envAsStrVec);
	return env;
}

std::string	EnvironmentBuilder::formatAsHTTPVariable(const std::string& key, const std::string& value)
{
	std::string	formattedEnvKeyValue = "";

	// Replace hyphens with underscores and convert to uppercase
	std::string	formattedKey = key;
	for (size_t i = 0; i < key.length(); ++i) {
		if (key[i] == '-' || key[i] == ' ') {
			formattedKey[i] = '_';
		}
	}
	strToUpper(formattedKey);

	// Convert HTTP headers to CGI format: HTTP_HEADER_NAME
	if (formattedKey == "CONTENT_TYPE" || formattedKey == "CONTENT_LENGTH")
		formattedEnvKeyValue = formattedKey + "=" + value;
	else
		formattedEnvKeyValue = "HTTP_" + formattedKey + "=" + value;

	return (formattedEnvKeyValue);
}

std::string	EnvironmentBuilder::formatKeyValueIntoSingleString(const std::string& key, const std::string& value)
{
	std::string	formattedEnvKeyValue = key + "=" + value;
	return formattedEnvKeyValue;
}

void EnvironmentBuilder::addCGIEnvironment(std::vector<std::string> envAsStrVec, const Request& request)
{
	// Standard CGI variables
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_METHOD", request.getMethod()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_URI", request.getRequestedURL()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PROTOCOL", request.getProtocol()));

	// Server information
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_NAME", "localhost"));		// or from config
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PORT", "8080"));			// or from config
}
