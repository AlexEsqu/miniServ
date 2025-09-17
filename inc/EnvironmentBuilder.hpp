#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <cstring>

#include "Environment.hpp"

class Request;

class EnvironmentBuilder
{

private:

	std::map<std::string, std::string>	_requestEnvMap; //key : value

public:

	//----------------- CONSTRUCTORS ---------------------//

	EnvironmentBuilder();
	EnvironmentBuilder(const EnvironmentBuilder& copy);

	//----------------- DESTRUCTOR -----------------------//

	~EnvironmentBuilder();

	//------------------- OPERATORS ----------------------//

	EnvironmentBuilder& operator=(const EnvironmentBuilder& other);

	//--------------------- SETTERS ----------------------//

	void		cutFormatAddToEnv(std::string& keyValueString);
	void		setAsEnv(const std::string& key, const std::string& value);
	void		setAsHTTPVariable(const std::string& headerKey, const std::string& headerValue);
	void		setupCGIEnvironment(const Request& request);

	//---------------------- GETTERS ---------------------//

	Environment	getPHPEnv();
	// Environment	getPythonEnv();
	// Environment	getEnvAsString();
	std::string	getSpecificEnv(std::string& key);

	//--------------- MEMBER FUNCTIONS -------------------//

};
