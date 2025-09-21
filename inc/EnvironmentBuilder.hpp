#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cstring>

#include "Environment.hpp"
#include "Request.hpp"

class EnvironmentBuilder
{

private:

	std::map<std::string, std::string>	_envAsMap; //key : value
	Request								_request;

public:

	//----------------- CONSTRUCTORS ---------------------//

	EnvironmentBuilder(Request& req);
	EnvironmentBuilder(const EnvironmentBuilder&);

	//----------------- DESTRUCTOR -----------------------//

	~EnvironmentBuilder();

	//------------------- OPERATORS ----------------------//

	EnvironmentBuilder&	operator=(const EnvironmentBuilder&);

	//--------------------- SETTERS ----------------------//

	void		setAsEnv(const std::string& key, const std::string& value);

	//---------------------- GETTERS ---------------------//

	std::string	getSpecificEnv(std::string& key);

	//--------------- MEMBER FUNCTIONS -------------------//

	std::string	formatKeyValueIntoSingleString(const std::string& key, const std::string& value);
	std::string	formatAsHTTPVariable(const std::string& headerKey, const std::string& headerValue);
	void		addCGIEnvironment(std::vector<std::string> envAsStrVec, const Request& request);
	Environment	generatePHPEnv();

};
