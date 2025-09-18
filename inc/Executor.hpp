#pragma once

#include "EnvironmentBuilder.hpp"
#include "Response.hpp"

class Executor
{

private:

	//------------------ ATTRIBUTES ----------------------//

	Environment	env;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Executor();
	Executor(Response& response);
	Executor(const Executor &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Executor();

	//-------------------- SETTER ------------------------//


	//-------------------- GETTERS -----------------------//


	//------------------- OPERATORS ----------------------//

	Executor &operator=(const Executor &other);

	//--------------- MEMBER FUNCTIONS -------------------//


};
