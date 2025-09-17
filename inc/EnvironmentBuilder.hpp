#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <cstring>

#include "Environment.hpp"

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

	void		setAdditionalEnv(std::string& keyValueString);
	void		setAdditionalEnv(const std::string& key, const std::string& value);

	//---------------------- GETTERS ---------------------//

	Environment	getPHPEnv();
	// Environment	getPythonEnv();
	// Environment	getEnvAsString();
	std::string	getSpecificEnv(std::string& key);

	//--------------- MEMBER FUNCTIONS -------------------//

};
