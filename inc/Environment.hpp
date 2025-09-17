#pragma once
#include <iostream>
#include <vector>
#include <cstring>

class Environment
{

private:

	std::vector<std::string>			_envAsStrings;
	std::vector<char*>					_envAsCharPtr; // temp value

public:

	//----------------- CONSTRUCTORS ---------------------//

	Environment(std::vector<std::string>& envAsStringVect);
	Environment(const Environment& copy);

	//----------------- DESTRUCTOR -----------------------//

	~Environment();

	//------------------- OPERATORS ----------------------//

	Environment& operator=(const Environment& other);

	//--------------------- SETTERS ----------------------//

	//---------------------- GETTERS ---------------------//

	char**	getEnv();

	//--------------- MEMBER FUNCTIONS -------------------//

};
