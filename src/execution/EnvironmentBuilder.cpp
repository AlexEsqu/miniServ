#include "EnvironmentBuilder.hpp"

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

void	EnvironmentBuilder::setAdditionalEnv(std::string& keyValueString)
{
	size_t equalPos = keyValueString.find(':');
	if (equalPos != std::string::npos) {
		std::string key = keyValueString.substr(0, equalPos);
		std::string value = keyValueString.substr(equalPos + 1);
		_requestEnvMap[key] = value;
	}
}

void	EnvironmentBuilder::setAdditionalEnv(const std::string& key, const std::string& value)
{
	_requestEnvMap[key] = value;
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
