#pragma once

#include "Executor.hpp"

//----------------- CONSTRUCTORS ---------------------//

Executor::Executor(Response& response)
	: env(response.getRequestEnvironment())
{

}

Executor::Executor(const Executor &original)
{
	*this = original;
}

//----------------- DESTRUCTOR -----------------------//

Executor::~Executor()
{

}

//------------------- OPERATORS ----------------------//

Executor& Executor::operator=(const Executor &original)
{
	if (this == &original)
		return *this;

	env = original.env;
	return *this;
}

//------------------- SETTERS ------------------------//

//------------------- GETTERS ------------------------//


